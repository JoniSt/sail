#ifndef SAIL_PLUGIN_H
#define SAIL_PLUGIN_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_string_node;

/*
 * A structure representing plugin information.
 */
struct sail_plugin_info {

    /*
     * The plugin loader will use the plugin's layout version to correctly handle the plugin. Unsupported
     * plugin layout versions will be reported. This field must be the very first key in a plugin information
     * file.
     *
     * Plugin layout is a list of exported functions. We use plugin layout versions to implement
     * backward compatibility in a simple and maintanable way.
     */
    int layout;

    /* Plugin version. For example: "1.5.2". */
    char *version;

    /* Plugin description. For example: "JPEG image". */
    char *description;

    /* A linked list of supported file extensions. For example: "jpg", "jpeg". */
    struct sail_string_node *extension_node;

    /* A linked list of supported mime types. For example: "image/jpeg". */
    struct sail_string_node *mime_type_node;

    /*
     * Magic identifier as a regex string. For example: "GIF8[79]a".
     *
     * See https://en.wikipedia.org/wiki/List_of_file_signatures for more.
     */
    char *magic;
};

typedef struct sail_plugin_info sail_plugin_info_t;

/*
 * A structure representing a plugin information linked list.
 */
struct sail_plugin_info_node {

    /* Plugin information. */
    struct sail_plugin_info *plugin_info;

    /* Full path to the plugin. */
    char *path;

    struct sail_plugin_info_node *next;
};

typedef struct sail_plugin_info_node sail_plugin_info_node_t;

/*
 * Plugin info functions.
 */

/*
 * Allocates a new plugin info object. The assigned plugin info MUST be destroyed later
 * with sail_destroy_plugin_info().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_plugin_info(struct sail_plugin_info **plugin_info);

/*
 * Destroys the specified plugin_info and all its internal allocated memory buffers.
 * The "plugin_info" pointer MUST NOT be used after calling this function.
 */
SAIL_EXPORT void sail_destroy_plugin_info(struct sail_plugin_info *plugin_info);

/*
 * Allocates a new plugin info node. The assigned node MUST be destroyed later with sail_destroy_plugin_info_node().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_plugin_info_node(struct sail_plugin_info_node **plugin_info_node);

/*
 * Destroys the specified plugin info node and all its internal allocated memory buffers.
 */
SAIL_EXPORT void sail_destroy_plugin_info_node(struct sail_plugin_info_node *plugin_info_node);

/*
 * Destroys the specified plugin info node and all its internal allocated memory buffers.
 * Repeats the destruction procedure recursively for the stored next pointer.
 */
SAIL_EXPORT void sail_destroy_plugin_info_node_chain(struct sail_plugin_info_node *plugin_info_node);

/*
 * Reads SAIL plugin info from the specified file and stores the parsed information into
 * the specified plugin info object. The assigned plugin info MUST be destroyed later
 * with sail_destroy_plugin_info().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_read_info(const char *file, struct sail_plugin_info **plugin_info);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
