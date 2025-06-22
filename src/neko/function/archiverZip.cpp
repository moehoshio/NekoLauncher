#include "neko/function/archiver.hpp"
#include "neko/schema/exception.hpp"
#include "neko/function/pattern.hpp"

#include <minizip-ng/mz.h>
#include <minizip-ng/mz_strm.h>
#include <minizip-ng/mz_zip.h>
#include <minizip-ng/mz_zip_rw.h>

#include <cstring>
#include <filesystem>


namespace neko::archive::zip {

    // RAII for minizip-ng zip reader
    class ZipReader {
        void *handle = nullptr;
        bool closed = false;

    public:
        ZipReader() {
            handle = mz_zip_reader_create();
            if (!handle)
                throw ex::Runtime("Failed to create zip reader");
        }
        ~ZipReader() {
            if (handle) {
                if (closed) {
                    mz_zip_reader_close(handle);
                    closed = true;
                }
                mz_zip_reader_delete(&handle);
            }
        }
        void close() {
            if (handle && !closed) {
                mz_zip_reader_close(handle);
                closed = true;
            }
        }
        void *get() { return handle; }
        const void *get() const { return handle; }
    };

    // RAII for minizip-ng zip writer
    class ZipWriter {
        void *handle = nullptr;
        bool closed = false;

    public:
        ZipWriter() {
            handle = mz_zip_writer_create();
            if (!handle)
                throw ex::Runtime("Failed to create zip writer");
        }
        ~ZipWriter() {
            if (handle) {
                if (closed) {
                    mz_zip_writer_close(handle);
                    closed = true;
                }
                mz_zip_writer_delete(&handle);
            }
        }
        void close() {
            if (handle && !closed) {
                mz_zip_writer_close(handle);
                closed = true;
            }
        }
        void *get() { return handle; }
        const void *get() const { return handle; }
    };

    void extract(const ExtractConfig &config) {
        ZipReader reader;
        neko::int32 err = mz_zip_reader_open_file(reader.get(), config.inputArchivePath.c_str());
        if (err != MZ_OK)
            throw ex::FileError("Failed to open zip file for reading: " + config.inputArchivePath, ex::ExceptionExtensionInfo{});

        if (!config.password.empty()) {
            mz_zip_reader_set_password(reader.get(), config.password.c_str());
        }

        neko::int32 entry_status = mz_zip_reader_goto_first_entry(reader.get());
        while (entry_status == MZ_OK) {
            mz_zip_file *file_info = nullptr;
            mz_zip_reader_entry_get_info(reader.get(), &file_info);

            std::string filename = file_info && (file_info->filename) ? file_info->filename : "";
            // Process includePaths/excludePaths
            bool skip = false;
            if (util::pattern::matchAny(filename, config.excludePaths)) {
                skip = true;
            }
            if (!config.includePaths.empty() && !util::pattern::matchAny(filename, config.includePaths)) {
                skip = true;
            }
            if (!skip) {
                std::string out_path = (std::filesystem::path(config.destDir) / filename).string();
                if (file_info && (file_info->flag & MZ_ZIP_FLAG_ENCRYPTED) && config.password.empty())
                    throw ex::FileError("Encrypted file requires password: " + filename, ex::ExceptionExtensionInfo{});

                if (file_info && file_info->filename[strlen(file_info->filename) - 1] == '/') {
                    std::filesystem::create_directories(out_path);
                } else {
                    if (std::filesystem::exists(out_path) && !config.overwrite) {
                        // Skip existing file if overwrite is false
                        entry_status = mz_zip_reader_goto_next_entry(reader.get());
                        continue;
                    }
                    std::filesystem::create_directories(std::filesystem::path(out_path).parent_path());
                    // Extract file
                    err = mz_zip_reader_entry_save_file(reader.get(), out_path.c_str());
                    if (err != MZ_OK)
                        throw ex::FileError("Failed to extract file: " + filename, ex::ExceptionExtensionInfo{});
                }
            }
            entry_status = mz_zip_reader_goto_next_entry(reader.get());
        }
    }

    void create(const CreateConfig &config) {
        ZipWriter writer;
        neko::int32 err = mz_zip_writer_open_file(writer.get(), config.outputArchivePath.c_str(), 0, 0);
        if (err != MZ_OK)
            throw ex::FileError("Failed to open zip file for writing: " + config.outputArchivePath, ex::ExceptionExtensionInfo{});

        if (!config.password.empty()) {
            mz_zip_writer_set_password(writer.get(), config.password.c_str());
            switch (config.encryption) {
                case ZipEncryption::AES256:
                    mz_zip_writer_set_aes(writer.get(), 1);
                    break;
                case ZipEncryption::ZipCrypto:
                default:
                    mz_zip_writer_set_aes(writer.get(), 0);
                    break;
            }
        }

        for (const auto &input : config.inputPaths) {
            if (std::filesystem::is_directory(input)) {
                auto inputAbs = std::filesystem::absolute(input);
                auto baseParent = inputAbs.parent_path();
                for (const auto &p : std::filesystem::recursive_directory_iterator(input)) {
                    if (!std::filesystem::is_regular_file(p))
                        continue;
                    std::string filePath = std::filesystem::relative(p.path(), baseParent).string();
                    if (util::pattern::matchAny(filePath, config.excludePaths))
                        continue;
                    err = mz_zip_writer_add_file(writer.get(), p.path().string().c_str(), filePath.c_str());
                    if (err != MZ_OK)
                        throw ex::FileError("Failed to add file: " + filePath + " in zip: " + config.outputArchivePath, ex::ExceptionExtensionInfo{});
                }
            } else {
                std::string filePath = std::filesystem::path(input).filename().string();
                if (util::pattern::matchAny(filePath, config.excludePaths))
                    continue;
                err = mz_zip_writer_add_file(writer.get(), input.c_str(), filePath.c_str());
                if (err != MZ_OK)
                    throw ex::FileError("Failed to add file: " + filePath + " in zip: " + config.outputArchivePath, ex::ExceptionExtensionInfo{});
            }
        }
    }

} // namespace neko::archive::zip
