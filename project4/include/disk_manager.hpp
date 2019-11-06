#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP

#include <cstdio>
#include <cstdint>
#include <string>

#include "headers.hpp"
#include "status.hpp"

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

    /// Create new page.
    pagenum_t page_create() const;

    /// Release page and add to free page list.
    /// \param pagenum pagenum_t, page ID.
    /// \return Status, whether success or not.
    Status page_free(pagenum_t pagenum) const;

    /// Read page from file.
    /// \param pagenum pagenum_t, page ID.
    /// \param dst Page*, pointer to write read page.
    /// \return Status, whether success or not.
    Status page_read(pagenum_t pagenum, Page* dst) const;

    /// Write page to file.
    /// \param pagenum pagenum_t, page ID.
    /// \param src Page const*, target page.
    /// \return Status, whether success or not.
    Status page_write(pagenum_t pagenum, Page const* src) const;

private:
    /// File pointer.
    FILE* fp;

    /// Initialize file.
    /// \return Status, whether success or not.
    Status file_init();

    /// Create file with given filename.
    /// \return Status, whether success or not.
    Status file_create(std::string const& filename);

    /// Read-write callback for abstracted page api.
    /// \param T callback type, Status(Page*).
    /// \param pagenum pagenum_t, page ID.
    /// \param func T, callback.
    /// \return Status, whether success or not.
    template <typename T>
    Status rwcallback(pagenum_t pagenum, T&& func) const {
        Page page;
        CHECK_SUCCESS(page_read(pagenum, &page));
        CHECK_SUCCESS(func(&page));
        CHECK_SUCCESS(page_write(pagenum, &page));
        return Status::SUCCESS;
    }
};

#endif