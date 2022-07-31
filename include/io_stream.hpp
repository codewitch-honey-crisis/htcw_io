#ifndef HTCW_STREAM_HPP
#define HTCW_STREAM_HPP
#include <htcw_endian.hpp>
#include <string.h>
#ifdef ARDUINO
#include <Arduino.h>
#ifdef SAMD_SERIES
#ifndef IO_SEEED_FS
#define IO_NO_FS
#endif
#endif
#ifdef ARDUINO_ARCH_STM32
#define IO_NO_FS

#endif
#if defined(ARDUINO_ARDUINO_NANO33BLE) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
#define IO_NO_FS
#endif
#ifndef IO_NO_FS
#ifdef IO_ARDUINO_SD_FS
#include <SD.h>
#else
#if !defined(SAMD_SERIES)
#include <FS.h>
#endif
#endif
#endif
#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP8266)
#include <pgmspace.h>
#elif defined(ARDUINO_ARDUINO_NANO33BLE) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include <api/deprecated-avr-comp/avr/pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#else
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#endif
#if defined(SAMD_SERIES) && defined(IO_SEEED_FS)
#include <Seeed_FS.h>

#include "SD/Seeed_SD.h"
#endif
namespace io {
struct stream_caps {
    uint8_t read : 1;
    uint8_t write : 1;
    uint8_t seek : 1;
};
enum struct seek_origin {
    start = 0,
    current = 1,
    end = 2
};
class stream {
   public:
    virtual int getch() = 0;
    virtual size_t read(uint8_t* destination, size_t size) = 0;
    virtual int putch(int value) = 0;
    virtual size_t write(const uint8_t* source, size_t size) = 0;
    virtual unsigned long long seek(long long position, seek_origin origin = seek_origin::start) = 0;
    virtual stream_caps caps() const = 0;
    template <typename T>
    size_t read(T* out_value) {
        return read((uint8_t*)out_value, sizeof(T));
    }
    template <typename T>
    T read(const T& default_value = T()) {
        T result;
        if (sizeof(T) != read(&result))
            return default_value;
        return result;
    }
    template <typename T>
    size_t write(const T& value) {
        return write((uint8_t*)&value, sizeof(T));
    }
};
enum struct file_mode {
    read = 1,
    write = 2,
    append = 6
};
class buffer_stream final : public stream {
    uint8_t* m_begin;
    uint8_t* m_current;
    size_t m_size;
    buffer_stream(const buffer_stream& rhs) = delete;
    buffer_stream& operator=(const buffer_stream& rhs) = delete;

   public:
    buffer_stream();
    buffer_stream(uint8_t* buffer, size_t size);
    buffer_stream(buffer_stream&& rhs);
    buffer_stream& operator=(buffer_stream&& rhs);
    uint8_t* handle();
    void set(uint8_t* buffer, size_t size);
    virtual int getch();
    virtual int putch(int value);
    virtual stream_caps caps() const;
    virtual size_t read(uint8_t* destination, size_t size);
    virtual size_t write(const uint8_t* source, size_t size);
    virtual unsigned long long seek(long long position, seek_origin origin = seek_origin::start);
};
class const_buffer_stream final : public stream {
    const uint8_t* m_begin;
    const uint8_t* m_current;
    size_t m_size;
    const_buffer_stream(const const_buffer_stream& rhs) = delete;
    const_buffer_stream& operator=(const const_buffer_stream& rhs) = delete;

   public:
    const_buffer_stream();
    const_buffer_stream(const uint8_t* buffer, size_t size);
    const_buffer_stream(const_buffer_stream&& rhs);
    const_buffer_stream& operator=(const_buffer_stream&& rhs);
    const uint8_t* handle() const;
    void set(const uint8_t* buffer, size_t size);
    virtual int getch();
    virtual int putch(int value);
    virtual stream_caps caps() const;
    virtual size_t read(uint8_t* destination, size_t size);
    virtual size_t write(const uint8_t* source, size_t size);
    virtual unsigned long long seek(long long position, seek_origin origin = seek_origin::start);
};
#ifdef ARDUINO

class arduino_stream final : public stream {
    Stream* m_stream;
    arduino_stream(const arduino_stream& rhs) = delete;
    arduino_stream& operator=(const arduino_stream& rhs) = delete;

   public:
    arduino_stream(Stream* stream);
    arduino_stream(arduino_stream&& rhs);
    arduino_stream& operator=(arduino_stream&& rhs);
    virtual size_t read(uint8_t* destination, size_t size);
    virtual int getch();
    virtual size_t write(const uint8_t* source, size_t size);
    virtual int putch(int value);
    virtual unsigned long long seek(long long position, seek_origin origin = seek_origin::start);
    virtual stream_caps caps() const;
};
#endif
#ifndef IO_NO_FS
class file_stream final : public stream {
#ifdef ARDUINO
    File& m_file;
#else
    FILE* m_fd;
#endif
    stream_caps m_caps;
    file_stream(const file_stream& rhs) = delete;
    file_stream& operator=(const file_stream& rhs) = delete;

   public:
#ifdef ARDUINO
    file_stream(File& file);
    file_stream(file_stream&& rhs);
    file_stream& operator=(file_stream&& rhs);
    ~file_stream();
    File& handle() const;
    void set(File& file);
    virtual size_t read(uint8_t* destination, size_t size);
    virtual int getch();
    virtual size_t write(const uint8_t* source, size_t size);
    virtual int putch(int value);
    virtual unsigned long long seek(long long position, seek_origin origin = seek_origin::start);
    void close();
#else
    file_stream(const char* name, file_mode mode = file_mode::read);
    file_stream(file_stream&& rhs);
    file_stream& operator=(file_stream&& rhs);
    ~file_stream();
    FILE* handle() const;
    void set(const char* name, file_mode mode = file_mode::read);
    virtual size_t read(uint8_t* destination, size_t size);
    virtual int getch();
    virtual size_t write(const uint8_t* source, size_t size);
    virtual int putch(int value);
    virtual unsigned long long seek(long long position, seek_origin origin = seek_origin::start);
    void close();
#endif
    virtual stream_caps caps() const;
};
#endif
class stream_reader_base {
   protected:
    stream* m_stream;
    stream_reader_base(io::stream* stream) : m_stream(stream) {
    }

   public:
    inline bool initialized() const {
        return m_stream != nullptr;
    }
    inline stream* base_stream() const {
        return m_stream;
    }
};
class stream_reader_le : public stream_reader_base {
   public:
    stream_reader_le(io::stream* stream);
    bool read(uint8_t* out_value) const;
    bool read(uint16_t* out_value) const;
    bool read(uint32_t* out_value) const;
#if HTCW_MAX_WORD >= 64
    bool read(uint64_t* out_value) const;
#endif
    bool read(int8_t* out_value) const;
    bool read(int16_t* out_value) const;
    bool read(int32_t* out_value) const;
#if HTCW_MAX_WORD >= 64
    bool read(int64_t* out_value) const;
#endif
};
class stream_reader_be : public stream_reader_base {
   public:
    stream_reader_be(io::stream* stream);
    bool read(uint8_t* out_value) const;
    bool read(uint16_t* out_value) const;
    bool read(uint32_t* out_value) const;
#if HTCW_MAX_WORD >= 64
    bool read(uint64_t* out_value) const;
#endif
    bool read(int8_t* out_value) const;
    bool read(int16_t* out_value) const;
    bool read(int32_t* out_value) const;
#if HTCW_MAX_WORD >= 64
    bool read(int64_t* out_value) const;
#endif
};

class buffered_read_stream final : public io::stream {
    io::stream& m_stream;
    void* (*m_allocator)(size_t);
    void (*m_deallocator)(void*);
    uint8_t* m_buffer;
    size_t m_buffer_capacity;
    size_t m_buffer_size;
    size_t m_buffer_index;
    void deallocate() {
        if (m_deallocator != nullptr && m_buffer != nullptr) {
            m_deallocator(m_buffer);
            m_buffer = nullptr;
        }
    }
    buffered_read_stream(const buffered_read_stream& rhs) = delete;
    buffered_read_stream& operator=(const buffered_read_stream& rhs) = delete;

   public:
    buffered_read_stream(io::stream* inner, size_t buffer_capacity = 1024, void*(allocator)(size_t) = ::malloc, void(deallocator)(void*) = ::free) : m_stream(*inner), m_allocator(allocator), m_deallocator(deallocator), m_buffer(nullptr), m_buffer_capacity(buffer_capacity), m_buffer_size(0), m_buffer_index(0) {
    }
    buffered_read_stream(buffered_read_stream&& rhs) : m_stream(rhs.m_stream) {
        m_allocator = rhs.m_allocator;
        m_deallocator = rhs.m_deallocator;
        m_buffer = rhs.m_buffer;
        rhs.m_buffer = nullptr;
        m_buffer_capacity = rhs.m_buffer_capacity;
        m_buffer_size = rhs.m_buffer_size;
        rhs.m_buffer_size = 0;
        m_buffer_index = rhs.m_buffer_index;
    }
    buffered_read_stream& operator=(buffered_read_stream&& rhs) {
        deallocate();
        m_stream = rhs.m_stream;
        m_allocator = rhs.m_allocator;
        m_deallocator = rhs.m_deallocator;
        m_buffer = rhs.m_buffer;
        rhs.m_buffer = nullptr;
        m_buffer_capacity = rhs.m_buffer_capacity;
        m_buffer_size = rhs.m_buffer_size;
        rhs.m_buffer_size = 0;
        m_buffer_index = rhs.m_buffer_index;
        return *this;
    }
    ~buffered_read_stream() {
        deallocate();
    }
    bool initialize() {
        if (m_buffer == nullptr) {
            m_buffer = (uint8_t*)m_allocator(m_buffer_capacity);
        }
        return m_buffer != nullptr;
    }
    inline bool initialized() const { return m_buffer != nullptr; }
    void set(io::stream* inner) {
        if (nullptr == inner) {
            return;
        }
        m_stream = *inner;
        m_buffer_size = 0;
        m_buffer_index = 0;
    }
    virtual int getch() {
        uint8_t result;
        if (1 != read(&result, 1)) {
            return -1;
        }
        return result;
    }
    virtual size_t read(uint8_t* destination, size_t size) {
        if (!initialize()) {
            return 0;
        }
        if (!m_buffer_size || m_buffer_index >= m_buffer_capacity) {
            m_buffer_size = m_stream.read(m_buffer, m_buffer_capacity);
            m_buffer_index = 0;
        }
        int sz = (m_buffer_size - m_buffer_index) < size ? m_buffer_size - m_buffer_index : size;
        if (sz > 0) {
            memcpy(destination, m_buffer + m_buffer_index, sz);
            m_buffer_index += sz;
            if (sz != size) {
                sz += read(destination + sz, size - sz);
            }
        }
        return sz;
    }
    virtual int putch(int value) {
        return -1;
    }
    virtual size_t write(const uint8_t* source, size_t size) {
        return 0;
    }
    virtual unsigned long long seek(long long position, io::seek_origin origin = io::seek_origin::start) {
        if (!m_stream.caps().seek) return 0;
        if (!initialize()) {
            return 0;
        }
        if (position > 0 && origin == io::seek_origin::current && position < (m_buffer_size - m_buffer_index)) {
            m_buffer_index += position;
            return m_stream.seek(0, io::seek_origin::current) + position;
        } else if (position != 0 || origin != io::seek_origin::current) {
            m_buffer_size = 0;
            m_buffer_index = 0;
        }
        return m_stream.seek(position, origin) + m_buffer_index;
    }
    virtual io::stream_caps caps() const {
        io::stream_caps result = m_stream.caps();
        result.write = false;
        return result;
    }
};
}  // namespace io
#endif