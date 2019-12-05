#include <atomic>
#include <cstring>
#include <chrono>
#include <random>
#include <thread>

#include "dbms.hpp"

struct Seq {
    enum class Cat { FIND, UPDATE } cat;
    prikey_t key;
};

int main() {
    Database dbms(100000, false);
    tableid_t tid = dbms.open_table("database1.db");

    constexpr size_t NUM_THREAD = 4;
    constexpr size_t MAX_TRX = 1000;
    constexpr size_t MAX_TRXSEQ = 1000;
    constexpr size_t MAX_KEY_RANGE = 100000;
    constexpr size_t MAX_ABORTS = 10;

    std::atomic<int> ntrxs(0);
    std::atomic<int> nquery(0);
    std::atomic<int> updates(0);
    std::atomic<int> aborts(0);

    Record record;
    std::strncpy(reinterpret_cast<char*>(record.value), "hello", 10);

    std::thread threads[NUM_THREAD];
    for (int i = 0; i < NUM_THREAD; ++i) {
        threads[i] = std::thread([&] {
            std::random_device rd;
            std::default_random_engine gen(rd());

            int num_trx = gen() % MAX_TRX;
            ntrxs += num_trx;

            for (int j = 0; j < num_trx; ++j) {
                Seq seqs[MAX_TRXSEQ];
                int num_seq = gen() % MAX_TRXSEQ;
                for (int k = 0; k < num_seq; ++k) {
                    prikey_t key = gen() % MAX_KEY_RANGE;
                    if (gen() % 2 == 0) {
                        seqs[k] = Seq{ Seq::Cat::FIND, key };
                    } else {
                        seqs[k] = Seq{ Seq::Cat::UPDATE, key };
                    }
                }

                trxid_t xid = dbms.begin_trx();
                TrxState state = dbms.trx_state(xid);
                for (int k = 0; k < num_seq && state == TrxState::RUNNING; ++k) {
                    ++nquery;
                    if (seqs[k].cat == Seq::Cat::FIND) {
                        dbms.find(tid, seqs[k].key, nullptr, xid);
                    } else {
                        ++updates;
                        dbms.update(tid, seqs[k].key, record, xid);
                    }
                    state = dbms.trx_state(xid);
                }
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

    return 0;
}