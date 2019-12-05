#include <atomic>
#include <cstring>
#include <chrono>
#include <random>
#include <thread>

#include "dbms.hpp"

constexpr size_t NUM_THREAD = 4;
constexpr size_t MAX_TRX = 1000;
constexpr size_t MAX_TRXSEQ = 1000;
constexpr size_t MAX_KEY_RANGE = 100000;
constexpr size_t MAX_ABORTS = 10;

struct Seq {
    enum class Cat { FIND, UPDATE } cat;
    prikey_t key;

    static int updates[NUM_THREAD][MAX_TRX + 1][MAX_TRXSEQ + 1];
};

int Seq::updates[NUM_THREAD][MAX_TRX + 1][MAX_TRXSEQ + 1];

int main() {
    Database dbms(100000, false);
    tableid_t tid = dbms.open_table("database1.db");

    std::vector<Record> keys = dbms.find_range(
        tid,
        std::numeric_limits<prikey_t>::min(),
        std::numeric_limits<prikey_t>::max());
    int key_range = std::min(MAX_KEY_RANGE, keys.size());

    std::atomic<int> ntrxs(0);
    std::atomic<int> nquery(0);
    std::atomic<int> updates(0);
    std::atomic<int> aborts(0);

    std::thread threads[NUM_THREAD];
    for (int i = 0; i < NUM_THREAD; ++i) {
        threads[i] = std::thread([&, i] {
            std::random_device rd;
            std::default_random_engine gen(rd());

            int num_trx = gen() % MAX_TRX;
            ntrxs += num_trx;

            Seq::updates[i][MAX_TRX][0] = num_trx;
            for (int j = 0; j < num_trx; ++j) {
                Seq seqs[MAX_TRXSEQ];
                int num_seq = gen() % MAX_TRXSEQ;
                for (int k = 0; k < num_seq; ++k) {
                    prikey_t key = keys[gen() % key_range].key;
                    if (gen() % 2 == 0) {
                        seqs[k] = Seq{ Seq::Cat::FIND, key };
                    } else {
                        seqs[k] = Seq{ Seq::Cat::UPDATE, key };
                    }
                }

                int idx = 0;
                trxid_t xid = dbms.begin_trx();
                TrxState state = dbms.trx_state(xid);
                for (int k = 0; k < num_seq && state == TrxState::RUNNING; ++k) {
                    ++nquery;
                    if (seqs[k].cat == Seq::Cat::FIND) {
                        dbms.find(tid, seqs[k].key, nullptr, xid);
                    } else {
                        ++updates;
                        Record rec;
                        std::snprintf(
                            reinterpret_cast<char*>(rec.value),
                            sizeof(Record) - sizeof(prikey_t),
                            "%lld value", seqs[k].key);
                        dbms.update(tid, seqs[k].key, rec, xid);
                        Seq::updates[i][j][idx++] = seqs[k].key;
                    }
                    state = dbms.trx_state(xid);
                }

                Seq::updates[i][j][MAX_TRXSEQ] = idx;

                dbms.end_trx(xid);
                if (state == TrxState::ABORTED) {
                    if (++aborts > MAX_ABORTS) {
                        break;
                    }
                }
            }
        });
    }

    bool runnable = true;
    std::thread logger([&] {
        while (runnable) {
            std::printf("\r%d", nquery.load());
        }
    });

    using namespace std::chrono;

    auto now = steady_clock::now();
    for (int i = 0; i < NUM_THREAD; ++i) {
        threads[i].join();
    }
    auto end = steady_clock::now();

    runnable = false;
    logger.join();

    // dbms.verbose(true);
    // dbms.print_tree(tid);

    size_t tick = duration_cast<milliseconds>(end - now).count();
    std::cout << '\r'
        << nquery.load() << " queris (" << updates.load() << " updates), "
        << ntrxs.load() << " trxs, "
        << tick << "ms (" << (static_cast<double>(nquery.load()) / tick) << "/ms, "
        << aborts.load() << " aborted)"
        << std::endl;

    for (int i = 0; i < NUM_THREAD; ++i) {
        int ntrx = Seq::updates[i][MAX_TRX][0];
        for (int j = 0; j < ntrx; ++j) {
            int nseq = Seq::updates[i][j][MAX_TRXSEQ];
            for (int k = 0; k < nseq; ++k) {
                prikey_t key = Seq::updates[i][j][k];

                Record rec;
                dbms.find(tid, key, &rec);

                auto str = std::to_string(k) + " value";
                if (std::strncmp(
                        reinterpret_cast<char*>(rec.value),
                        str.c_str(),
                        sizeof(Record) - sizeof(prikey_t))
                ) {
                    std::cout
                        << "\rtest failed: " << key
                        << ", " << reinterpret_cast<char*>(rec.value) << std::endl;
                    return 0;
                }
            }
        }
    }

    return 0;
}