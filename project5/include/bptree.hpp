#ifndef BPTREE_HPP
#define BPTREE_HPP

#include <vector>

#include "buffer_manager.hpp"
#include "disk_manager.hpp"
#include "headers.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

/// Read-write specialization.
namespace _RWSpec {
    /// Access type.
    enum class Access {
        READ = 0,
        WRITE = 1,
    };

    /// Compile time method selection.
    /// \param Access constant, access type selection, default read.
    template <Access>
    struct Action {
        /// Run buffer in read mode.
        /// \param F typename, Status(Record const&).
        /// \param buf Ubuffer&, buffer.
        /// \param callback F&&, callback method.
        /// \return auto, return value of callback.
        template <typename F>
        static auto run(Ubuffer& buf, F&& callback) {
            return buf.read(std::forward<F>(callback));
        }
    };

    /// Write access specialization.
    template <>
    struct Action<Access::WRITE> {
        /// Run buffer in write mode.
        /// \param F typename, Status(Record&).
        /// \param buf Ubuffer&, buffer.
        /// \param callback F&&, callback method.
        /// \return auto, return value of callback.
        template <typename F>
        static auto run(Ubuffer& buf, F&& callback) {
            return buf.write(std::forward<F>(callback));
        }
    };
};
        
/// B+Tree record iterator.
class BPTreeIterator;

/// B+Tree Structure.
class BPTree {
public:
    /// default lear order, 32
    static constexpr int DEFAULT_LEAF_ORDER = 32;

    /// default internal order, 249
    static constexpr int DEFAULT_INTERNAL_ORDER = 249;

    /// default value for whether use delayed merge or not, true
    static constexpr bool DEFAULT_DELAYED_MERGE = true;

    /// Construct on-disk b+tree with given file and buffers.
    /// \param file FileManager*, file base.
    /// \param buffers BufferManager*, buffer manager.
    BPTree(FileManager* file, BufferManager* buffers);

    /// Default destructor.
    ~BPTree() = default;

    /// Removed copy constructor.
    BPTree(BPTree const&) = delete;

    /// Move constructor.
    BPTree(BPTree&& other) noexcept;

    /// Removed copy assignment.
    BPTree& operator=(BPTree const&) = delete;

    /// Move assignment.
    BPTree& operator=(BPTree&& other) noexcept;

    /// Reset file manager pointer.
    /// \param new_file FileManager*, new file manager.
    void reset_file(FileManager* file);

    /// Update b+tree configuration.
    /// \param leaf_order int, leaf order.
    /// \param internal_order int, internal order.
    /// \param delayed_merge bool, whether use delayed merge or not.
    void test_config(int leaf_order, int internal_order, bool delayed_merge);

    /// Print all leaves.
    void print_leaves() const;

    /// Print tree.
    void print_tree() const;

    /// Find given key and write the result to argument record.
    /// \param key prikey_t, primary key.
    /// \param record Record*, nullable, pointer to write the result.
    /// \param xid trxid_t, transaction id, default INVALID_TRXID.
    /// \return Status, whether success to find or not exists.
    Status find(
        prikey_t key, Record* record, trxid_t xid = INVALID_TRXID) const;

    /// Range based search.
    /// \param start prikey_t, start point.
    /// \param end prikey_t, end point.
    /// \return std::vector<Record>, result sequence.
    std::vector<Record> find_range(prikey_t start, prikey_t end) const;

    /// Insert given key and value to tree.
    /// \param key prikey_t, primary key.
    /// \param value const uint8_t*, byte sequence.
    /// \param value_size int, the size of the value.
    /// \return Status, whether success to insert items or not.
    Status insert(prikey_t key, const uint8_t* value, int value_size) const;

    /// Remove records based on given key.
    /// \param key prikey_t, primary key.
    /// \return Status, whether success to remove proper items or not.
    Status remove(prikey_t key) const;

    /// Update record to given.
    /// \param key prikey_t, primary key.
    /// \param record Record, update value.
    /// \param xid trxid_t, transaction id, default INVALID_TRXID.
    /// \return Status, whether success to udpate records or not.
    Status update(
        prikey_t key, Record record, trxid_t xid = INVALID_TRXID) const;

    /// Remove all records and clean tree.
    /// \return Status, whether success to destroy tree or not.
    Status destroy_tree() const;

    /// Get the beginning of b+tree record iterator.
    BPTreeIterator begin() const;

    /// Get the end of b+tree record iterator. 
    BPTreeIterator end() const;

    /// Set database.
    Status set_database(Database& dbms);

private:
    int leaf_order;                 /// order of the leaf node.
    int internal_order;             /// internal of the leaf node.
    bool verbose_output;            /// whether verbose or not.
    bool delayed_merge;             /// use delayed merge or not.
    FileManager* file;              /// target file pointer.
    BufferManager* buffers;         /// buffer manager.
    Database* dbms;                 /// database pointer.

    friend class BPTreeIterator;

    using Access = _RWSpec::Access;

    /// Return buffer specified by pageid.
    /// \param pagenum pagenum_t, page id.
    /// \return Ubuffer, buffer.
    Ubuffer buffering(pagenum_t pagenum) const;

    /// Return buffer speicified by pid with lock acquirance.
    /// \param pagenum pagenum_t, page id.
    /// \param xid trxid_t, transaction id.
    /// \param mode LockMode, lock mode.
    /// \return Ubuffer, buffer.
    Ubuffer require_buffering(
        pagenum_t pagenum, trxid_t xid, LockMode mode) const;

    /// Create page on buffer.
    /// \param leaf bool, whether generated page is leaf page or internal.
    /// \return Ubuffer, created buffer.
    Ubuffer create_page(bool leaf) const;

    /// Release page.
    /// \param pagenum pagenum_t, page id.
    /// \return Status, whether success to free page or not.
    Status free_page(pagenum_t pagenum) const;

    /// Compute the path cost to the root.
    /// \param pagenum pagenum_t, page id.
    /// \return int, path cost.
    int path_to_root(pagenum_t pagenum) const;

    /// Compute split point based on length, ceil(length / 2).
    /// \param length int, base length.
    /// \return int, split point.
    static constexpr int cut(int length) {
        return length % 2 == 0
            ? length / 2
            : length / 2 + 1;
    }

    /// Find leaf with given key and write the result to given buffer.
    /// \param key prikey_t, primary key.
    /// \param buffer Ubuffer&, buffer to write the result.
    /// \return pagenum_t, found leaf page id.
    pagenum_t find_leaf(prikey_t key, Ubuffer& buffer) const;

    /// Find the key from the giveen buffer and run callback.
    /// \tparam T constant, Access, access token.
    /// \tparam F typename, Status(Record&), callback type.
    /// \param key prikey_t, primary key.
    /// \param buffer Ubuffer&, target buffer.
    /// \param callback F&&, callback for found record.
    /// \return int, index of the record in page.
    template <Access T, typename F>
    int find_key_from_leaf(
        prikey_t key, Ubuffer& buffer, F&& callback
    ) const {
        using PageT = std::conditional_t<T == Access::READ, Page const, Page>;
        return _RWSpec::Action<T>::run(buffer, [&](PageT& page) {
            if (!page.page_header().is_leaf) {
                return -1;
            }

            int i, num_key = page.page_header().number_of_keys;
            for (i = 0; i < num_key && page.records()[i].key != key; ++i)
                {}
            
            if (i < num_key
                && callback(page.records()[i]) == Status::SUCCESS
            ) {
                return i;
            }
            return -1;
        });
    }

    /// Find the key from the given buffer and write the result to `record`.
    /// \param key prikey_t, primary key.
    /// \param buffer Ubuffer&, target buffer.
    /// \param record Record*, pointer to write the found record.
    /// \return Status, whether find the key or not.
    Status find_key_from_leaf(
        prikey_t key, Ubuffer& buffer, Record* record) const;

    /// Find the page id from the given internal node and write the result to `idx`.
    /// \param pgaenum pagenum_t, page id.
    /// \param buffer Ubuffer&, target buffer.
    /// \param idx int&, reference for writing the index of the page id.
    /// \return Status, whether find the page id or not.
    Status find_pagenum_from_internal(
        pagenum_t pagenum, Ubuffer& buffer, int& idx) const;

    /// Write the record with given values.
    /// \param rec Record&, target record.
    /// \param key prikey_t, primary key.
    /// \param value const uint8_t*, byte sequence.
    /// \param value_size int, the size of the value sequence.
    /// \return Status, whether success to write the record or not.
    static Status write_record(
        Record& rec, prikey_t key, const uint8_t* value, int value_size);
    
    /// Insert the record to the given leaf.
    /// \param leaf Ubuffer, target leaf.
    /// \param rec Record const&, record for insertion.
    /// \return Status, whether success to write the record or not.
    Status insert_to_leaf(Ubuffer leaf, Record const& rec) const;

    /// Insert the record and split leaf node in two.
    /// \param leaf Ubuffer, target leaf.
    /// \param rec Record const&, record for insertion.
    /// \return Status, whether success to write the record or not.
    Status insert_and_split_leaf(Ubuffer leaf, Record const& rec) const;

    /// Insert the internal entry at given index of the node.
    /// \param node Ubuffer, target node.
    /// \param index int, insertion point.
    /// \param entry Internal const&, entry for insertion.
    /// \return Status, whether success to write the entry or not.
    Status insert_to_node(
        Ubuffer node, int index, Internal const& entry) const;

    /// Insert the entry and split the node in two.
    /// \param node Ubuffer, target node.
    /// \param index int, insertion point.
    /// \param entry Internal const&, entry for insertion.
    /// \return Status, whether success to write the entry or not.
    Status insert_and_split_node(
        Ubuffer node, int index, Internal const& entry) const;

    /// Insert the given key and two nodes to parent.
    /// \param left Ubuffer, left node.
    /// \param key prikey_t, intermediate key value.
    /// \param right Ubuffer, right node.
    /// \return Status, whether success to update parent or not.
    Status insert_to_parent(Ubuffer left, prikey_t key, Ubuffer right) const;

    /// Create new root and update file header.
    /// \param left Ubuffer, left node.
    /// \param key prikey_t, intermediate key value.
    /// \param right Ubuffer, right node.
    /// \return Status, whether success to create new root or not.
    Status insert_new_root(Ubuffer left, prikey_t key, Ubuffer right) const;

    /// Create new tree.
    /// \param rec Record const&, initial record.
    /// \return Status, whether success to create new tree or not.
    Status new_tree(Record const& rec) const;

    /// Remove record from the given leaf node.
    /// \param key prikey_t, primary key.
    /// \param node Ubuffer&, target leaf node.
    /// \return Status, whether success to remove the key or not.
    Status remove_record_from_leaf(prikey_t key, Ubuffer& node) const;

    /// Remove entry from the given internal node.
    /// \param key prikey_t, primary key.
    /// \param node Ubuffer&, target internal node.
    /// \return Status, whether success to remove the key or not.
    Status remove_entry_from_internal(prikey_t key, Ubuffer& node) const;

    /// Rotate the key to the right.
    /// \param left Ubuffer, left node.
    /// \param key prikey_t, key to rotate.
    /// \param index int, key index from the parent.
    /// \param right Ubuffer, right node.
    /// \param parent, Ubuffer, parent node.
    /// \return Status, whether success to rotate the key or not.
    Status rotate_to_right(
        Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
    ) const;
    
    /// Rotate the key to the left.
    /// \param left Ubuffer, left node.
    /// \param key prikey_t, key to rotate.
    /// \param index int, key index from the parent.
    /// \param right Ubuffer, right node.
    /// \param parent, Ubuffer, parent node.
    /// \return Status, whether success to rotate the key or not.
    Status rotate_to_left(
        Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
    ) const;
    
    /// Rearrange the root, whether remove root or make child as root or etc..
    /// \return Status, whether success to update the root.
    Status shrink_root() const;

    /// Merge two nodes in one.
    /// \param left Ubuffer, left node.
    /// \param key prikey_t, key between two nodes.
    /// \param right Ubuffer, right node.
    /// \param parent, Ubuffer, parent node.
    /// \return Status, whether success to merge or not.
    Status merge_nodes(
        Ubuffer left, prikey_t key, Ubuffer right, Ubuffer parent) const;
    
    /// Redistribute keys between two nodes.
    /// \param left Ubuffer, left node.
    /// \param key prikey_t, key to rotate.
    /// \param index int, key index from the parent.
    /// \param right Ubuffer, right node.
    /// \param parent, Ubuffer, parent node.
    /// \return Status, whether success to redistribute or not.
    Status redistribute_nodes(
        Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
    ) const;
    
    /// Delete internal key from the entry, propagate the effects to root.
    /// \param key prikey_t, key to delete.
    /// \param page Ubuffer, target page buffer.
    /// \return Status, whether success to delete entry or not.
    Status delete_entry(prikey_t key, Ubuffer page) const;

#ifdef TEST_MODULE
    friend struct BPTreeTest;
#endif
};

#endif