#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP

#include <cstdio>
#include <cstdint>
#include <string>

#include "headers.hpp"
#include "status.hpp"

using fileid_t = std::size_t;

/// File level disk manager.
class FileManager {
public:
    /// Default constructor.
    FileManager();

    /// Open or create file with given filename.
    /// \param filename std::string const&, the name of the file.
    FileManager(std::string const& filename);

    /// Destructor, free file pointer.
    ~FileManager();

    /// Default copy constructor, deleted.
    FileManager(FileManager const&) = delete;

    /// Move constructor.
    FileManager(FileManager&& other) noexcept;

    /// Default copy assignment, deleted.
    FileManager& operator=(FileManager const&) = delete;

    /// Move assignment.
    FileManager& operator=(FileManager&& other) noexcept;

    /// Get file id.
    fileid_t get_id() const;

    /// Rehashing id.
    fileid_t rehash();

    /// Set file id.
    fileid_t rehash(fileid_t id);

    /// Get filename.
    std::string const& get_filename() const;

    /// Hashing file name.
    /// \param filename std::string const&, the name of the file.
    /// \return std::pair<std::string, std::size_t>, preprocessed name and hash.
    static std::pair<std::string, std::size_t> hash_filename(
        std::string const& filename);

    /// Rehasing file id.
    /// \param id fileid_t, file ID.
    /// \return fileid_t, rehashed file id.
    static fileid_t rehash_fileid(fileid_t id);

    /// Create new page.
    pagenum_t page_create() const;

    /// Release page and add to free page list.
    /// \param pagenum pagenum_t, page ID.
    /// \return Status, whether success or not.
    Status page_free(pagenum_t pagenum) const;

    /// Read page from file.
    /// \param pagenum pagenum_t, page ID.
    /// \param dst Page&, pointer to write read page.
    /// \return Status, whether success or not.
    Status page_read(pagenum_t pagenum, Page& dst) const;

    /// Write page to file.
    /// \param pagenum pagenum_t, page ID.
    /// \param src Page const&, target page.
    /// \return Status, whether success or not.
    Status page_write(pagenum_t pagenum, Page const& src) const;

private:
    /// File pointer.
    FILE* fp;

    /// File ID.
    fileid_t id;

    /// file name.
    std::string name;

    /// Initialize file.
    /// \return Status, whether success or not.
    Status file_init();

    /// Create file with given filename.
    /// \return Status, whether success or not.
    Status file_create(std::string const& filename);

    /// Read-write callback for abstracted page api.
    /// \param T callback type, Status(Page&).
    /// \param pagenum pagenum_t, page ID.
    /// \param func T, callback.
    /// \return Status, whether success or not.
    template <typename T>
    inline Status page_callback(pagenum_t pagenum, T&& func) const {
        Page page;
        page_read(pagenum, page);
        CHECK_SUCCESS(func(page));
        CHECK_SUCCESS(page_write(pagenum, page));
        return Status::SUCCESS;
    }
};

#endif