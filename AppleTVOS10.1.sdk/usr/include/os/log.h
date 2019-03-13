/*
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __os_log_h
#define __os_log_h

#include <os/object.h>
#include <os/base.h>
#include <os/trace.h>
#include <stdint.h>
#include <stdbool.h>
#include <mach-o/loader.h>

#if !__has_builtin(__builtin_os_log_format)
#error using os/log.h requires Xcode 8 or later
#endif

__BEGIN_DECLS

#ifdef OS_LOG_FORMAT_WARNINGS
#define OS_LOG_FORMAT_ERRORS _Pragma("clang diagnostic warning \"-Wformat\"")
#else
#define OS_LOG_FORMAT_ERRORS _Pragma("clang diagnostic error \"-Wformat\"")
#endif

extern struct mach_header __dso_handle;

#if OS_OBJECT_SWIFT3
OS_OBJECT_DECL_SWIFT(os_log);
#elif OS_OBJECT_USE_OBJC
OS_OBJECT_DECL(os_log);
#else
typedef struct os_log_s *os_log_t;
#endif /* OS_OBJECT_USE_OBJC */

/*!
 * @const OS_LOG_DISABLED
 *
 * @discussion
 * Use this to disable a specific log message.
 */
#define OS_LOG_DISABLED NULL

/*!
 * @const OS_LOG_DEFAULT
 *
 * @discussion
 * Use this to log a message in accordance with current system settings.
 */
#define OS_LOG_DEFAULT OS_OBJECT_GLOBAL_OBJECT(os_log_t, _os_log_default)
__API_AVAILABLE(macosx(10.11), ios(9.0), watchos(2.0), tvos(9.0))
OS_EXPORT
struct os_log_s _os_log_default;

/*!
 * @enum os_log_type_t
 *
 * @discussion
 * Supported log message types.
 *
 * @constant OS_LOG_TYPE_DEFAULT
 * Equivalent type for "os_log()" messages, i.e., default messages that are always
 * captured to memory or disk.
 *
 * @constant OS_LOG_TYPE_INFO
 * Equivalent type for "os_log_info()" messages, i.e., Additional informational messages.
 *
 * @constant OS_LOG_TYPE_DEBUG
 * Equivalent type for "os_log_debug()" messages, i.e., Debug messages.
 *
 * @constant OS_LOG_TYPE_ERROR
 * Equivalent type for "os_log_error()" messages, i.e., local process error messages.
 *
 * @constant OS_LOG_TYPE_FAULT
 * Equivalent type for "os_log_fault()" messages, i.e., a system error that involves
 * potentially more than one process, usually used by daemons and services.
 */
OS_ENUM(os_log_type, uint8_t,
        OS_LOG_TYPE_DEFAULT = 0x00,
        OS_LOG_TYPE_INFO    = 0x01,
        OS_LOG_TYPE_DEBUG   = 0x02,
        OS_LOG_TYPE_ERROR   = 0x10,
        OS_LOG_TYPE_FAULT   = 0x11);

/*!
 * @function os_log_create
 *
 * @abstract
 * Creates a log object to be used with other log related functions.
 *
 * @discussion
 * Creates a log object to be used with other log related functions.  The
 * log object serves two purposes:  (1) tag related messages by subsystem
 * and category name for easy filtering, and (2) control logging system
 * behavior for messages. The log object may be NULL if logging is disabled
 * on the system.
 *
 * @param subsystem
 * The identifier of the given subsystem should be in reverse DNS form
 * (i.e., com.company.mysubsystem).
 *
 * @param category
 * The category within the given subsystem that specifies the settings for
 * the log object.
 *
 * @result
 * Returns an os_log_t value to be passed to other os_log API calls.  This
 * should be called once at log initialization and rely on system to detect
 * changes to settings.  This object should be released when no longer used
 * via os_release or -[release] method.
 *
 * A value will always be returned to allow for dynamic enablement.
 */
__API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT OS_OBJECT_RETURNS_RETAINED OS_NONNULL_ALL
os_log_t
os_log_create(const char *subsystem, const char *category);

/*!
 * @function os_log_info_enabled
 *
 * @abstract
 * Returns if additional information is enabled,
 *
 * @discussion
 * Returns if additional information is enabled,
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @result
 * Returns ‘true’ if debug log messages are enabled.
 */
#define os_log_info_enabled(log) os_log_type_enabled(log, OS_LOG_TYPE_INFO)

/*!
 * @function os_log_debug_enabled
 *
 * @abstract
 * Returns if debug log messages are enabled for a particular log object.
 *
 * @discussion
 * Returns if debug log messages are enabled for a particular log object.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @result
 * Returns ‘true’ if debug log messages are enabled.
 */
#define os_log_debug_enabled(log)  os_log_type_enabled(log, OS_LOG_TYPE_DEBUG)

/*!
 * @function os_log_with_type
 *
 * @abstract
 * Log a message using a specific type.
 *
 * @discussion
 * Will log a message with the provided os_log_type_t.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param type
 * Pass a valid type from os_log_type_t.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types in addition to %@ (objects).
 */
#define os_log_with_type(log, type, format, ...) __extension__({ \
    if (os_log_type_enabled(log, type)) { \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wvla\"") \
        OS_LOG_FORMAT_ERRORS \
        __attribute__((section("__TEXT,__oslogstring,cstring_literals"),internal_linkage)) static const char __format[] __asm(OS_STRINGIFY(OS_CONCAT(LOSLOG_, __COUNTER__))) = format; \
        uint8_t _os_log_buf[__builtin_os_log_format_buffer_size(format, ##__VA_ARGS__)]; \
        _os_log_impl(&__dso_handle, log, type, __format, (uint8_t *) __builtin_os_log_format(_os_log_buf, format, ##__VA_ARGS__), (unsigned int) sizeof(_os_log_buf)); \
        _Pragma("clang diagnostic pop") \
    } \
})

/*!
 * @function os_log
 *
 * @abstract
 * Insert a log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Insert a log message into the Unified Logging and Tracing system in 
 * accordance with the preferences specified by the provided log object.
 * These messages cannot be disabled and therefore always captured either
 * to memory or disk.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 1024 bytes per log line for dynamic content,
 * such as %s and %@, that can be written to the persistence store.
 * All content exceeding the limit will be truncated before it is
 * written to disk.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log(log, format, ...)    os_log_with_type(log, OS_LOG_TYPE_DEFAULT, format, ##__VA_ARGS__)

/*!
 * @function os_log_info
 *
 * @abstract
 * Insert an additional information log message into the Unified Logging
 * and Tracing system.
 *
 * @discussion
 * Insert a log message into the Unified Logging and Tracing system in 
 * accordance with the preferences specified by the provided log object.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_info(log, format, ...)   os_log_with_type(log, OS_LOG_TYPE_INFO, format, ##__VA_ARGS__)

/*!
 * @function os_log_debug
 *
 * @abstract
 * Insert a debug log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Insert a debug log message into the Unified Logging and Tracing system in
 * accordance with the preferences specified by the provided log object.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_debug(log, format, ...)  os_log_with_type(log, OS_LOG_TYPE_DEBUG, format, ##__VA_ARGS__)

/*!
 * @function os_log_error
 *
 * @abstract
 * Insert an error log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Insert an error log message into the Unified Logging and Tracing system.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_error(log, format, ...)  os_log_with_type(log, OS_LOG_TYPE_ERROR, format, ##__VA_ARGS__)

/*!
 * @function os_log_fault
 *
 * @abstract
 * Insert a fault log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Log a fault message issue into the Unified Logging and Tracing system
 * signifying a multi-process (i.e., system error) related issue, either
 * due to interaction via IPC or some other.  Faults will gather information 
 * from the entire process chain and record it for later inspection.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_fault(log, format, ...)  os_log_with_type(log, OS_LOG_TYPE_FAULT, format, ##__VA_ARGS__)

/*!
 * @function os_log_type_enabled
 *
 * @abstract
 * Evaluate if a specific log type is enabled before doing work
 *
 * @discussion
 * Evaluate if a specific log type is enabled before doing work
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param type
 * Pass a valid type from os_log_type_t.
 *
 * @result
 * Will return a boolean.
 */
__API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_log_type_enabled(os_log_t oslog, os_log_type_t type);

/*!
 * @function _os_log_impl
 *
 * @abstract
 * Internal function that takes compiler generated encoding and captures the necessary content.
 */
__API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
_os_log_impl(void *dso, os_log_t log, os_log_type_t type, const char *format, uint8_t *buf, unsigned int size);

/*
 * Support for older iteration of API for source compatibility only...
 */

__API_DEPRECATED("no longer supported, use os_log_debug(OS_LOG_DEFAULT, ...)", macosx(10.11,10.12), ios(9.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
_os_log_internal(void *dso, os_log_t log, os_log_type_t type, const char *message, ...);

__API_AVAILABLE(macosx(10.11), ios(9.0), watchos(2.0), tvos(9.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT OS_OBJECT_RETURNS_RETAINED OS_NOT_TAIL_CALLED OS_NONNULL_ALL
os_log_t
_os_log_create(void *dso, const char *subsystem, const char *category);

__API_DEPRECATED("no longer suppored - always returns true", macosx(10.11,10.12), ios(9.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_log_is_enabled(os_log_t log);

__API_DEPRECATED_WITH_REPLACEMENT("os_log_debug_enabled", macosx(10.11,10.12), ios(9.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_log_is_debug_enabled(os_log_t log);

__API_DEPRECATED("no longer supported - use os_log with per-parameter privacy options", macosx(10.11,10.12), ios(9.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0))
OS_NOTHROW OS_ALWAYS_INLINE
static inline void
_os_log_sensitive_deprecated(void) { }

#define os_log_sensitive(log, format, ...) __extension__({              \
    os_log_with_type(log, OS_LOG_TYPE_DEFAULT, format, ##__VA_ARGS__);  \
    _os_log_sensitive_deprecated();                                     \
})

#define os_log_sensitive_debug(log, format, ...) __extension__({        \
    os_log_with_type(log, OS_LOG_TYPE_DEBUG, format, ##__VA_ARGS__);    \
    _os_log_sensitive_deprecated();                                     \
})

#define OS_LOG_RELEASE OS_OBJECT_GLOBAL_OBJECT(os_log_t, _os_log_release)
__API_DEPRECATED("use os_log(OS_LOG_DEFAULT, ...)", macosx(10.11,10.11), ios(9.0,9.0), watchos(2.0,2.0), tvos(9.0,9.0))
OS_EXPORT
struct os_log_s _os_log_release;

#define OS_LOG_DEBUG OS_OBJECT_GLOBAL_OBJECT(os_log_t, _os_log_debug)
__API_DEPRECATED("use os_log_debug(OS_LOG_DEFAULT, ...)", macosx(10.11,10.11), ios(9.0,9.0), watchos(2.0,2.0), tvos(9.0,9.0))
OS_EXPORT
struct os_log_s _os_log_debug;

#define OS_LOG_ERROR OS_OBJECT_GLOBAL_OBJECT(os_log_t, _os_log_error)
__API_DEPRECATED("use os_log_error(OS_LOG_DEFAULT, ...)", macosx(10.11,10.11), ios(9.0,9.0), watchos(2.0,2.0), tvos(9.0,9.0))
OS_EXPORT
struct os_log_s _os_log_error;

#define OS_LOG_FAULT OS_OBJECT_GLOBAL_OBJECT(os_log_t, _os_log_fault)
__API_DEPRECATED("use os_log_fault(OS_LOG_DEFAULT, ...)", macosx(10.11,10.11), ios(9.0,9.0), watchos(2.0,2.0), tvos(9.0,9.0))
OS_EXPORT
struct os_log_s _os_log_fault;

#if ((defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_12) \
     || (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && !defined(__TV_OS_VERSION_MIN_REQUIRED) \
         && !defined(__WATCH_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_10_0) \
     || (defined(__TV_OS_VERSION_MIN_REQUIRED) && __TV_OS_VERSION_MIN_REQUIRED < __TVOS_10_0) \
     || (defined(__WATCH_OS_VERSION_MIN_REQUIRED) && __WATCH_OS_VERSION_MIN_REQUIRED < __WATCHOS_3_0))
#undef os_log_with_type
#define os_log_with_type(log, type, format, ...) __extension__({ \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic error \"-Wformat\"") \
    _os_log_internal(&__dso_handle, log, type, format, ##__VA_ARGS__); \
    _Pragma("clang diagnostic pop") \
})

#undef os_log_debug_enabled
#define os_log_debug_enabled(...) os_log_is_debug_enabled(__VA_ARGS__)

#undef os_log_create
#define os_log_create(subsystem, category) _os_log_create(&__dso_handle, subsystem, category)
#endif

__END_DECLS

#endif /* __os_log_h */
