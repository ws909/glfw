//========================================================================
// Window properties test
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdarg.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static char title_buffer[64] = "";
static char body_buffer[128] = "";
static char summary_buffer[256] = "";

static bool live_edit = false;

static bool title_supported = true;
static bool body_supported = true;
static bool summary_supported = true;

static bool use_index = false;

static int notification_count = 0;

static GLFWnotification* last_notification;

void sendNotification()
{
    if (use_index)
    {
        char title[sizeof(title_buffer)];
        sprintf(title, "%d", notification_count);
    
        const unsigned long buffer_length = strlen(title_buffer);
    
        if (buffer_length > 0)
        {
            strcat(title, ": ");
            strncat(title, title_buffer, buffer_length - strlen(title) - 1);
        }
        glfwSendNotification(title, body_buffer, summary_buffer);
    }
    else
        glfwSendNotification(title_buffer, body_buffer, summary_buffer);
    
    ++notification_count;
}

void editLastNotification()
{
    
}

void retractLastNotification()
{
    glfwNotificationRetract(last_notification);
}

int main(int argc, char** argv)
{
    int windowed_x, windowed_y, windowed_width, windowed_height;
    int last_xpos = INT_MIN, last_ypos = INT_MIN;
    int last_width = INT_MIN, last_height = INT_MIN;
    int limit_aspect_ratio = false, aspect_numer = 1, aspect_denom = 1;
    int limit_min_size = false, min_width = 400, min_height = 400;
    int limit_max_size = false, max_width = 400, max_height = 400;
    char width_buffer[12] = "", height_buffer[12] = "";
    char xpos_buffer[12] = "", ypos_buffer[12] = "";
    char numer_buffer[12] = "", denom_buffer[12] = "";
    char min_width_buffer[12] = "", min_height_buffer[12] = "";
    char max_width_buffer[12] = "", max_height_buffer[12] = "";
    int may_close = true;
    

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(600, 600, "Window Features", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);

    bool position_supported = true;

    glfwGetError(NULL);
    glfwGetWindowPos(window, &last_xpos, &last_ypos);
    sprintf(xpos_buffer, "%i", last_xpos);
    sprintf(ypos_buffer, "%i", last_ypos);
    if (glfwGetError(NULL) == GLFW_FEATURE_UNAVAILABLE)
        position_supported = false;

    glfwGetWindowSize(window, &last_width, &last_height);
    sprintf(width_buffer, "%i", last_width);
    sprintf(height_buffer, "%i", last_height);

    sprintf(numer_buffer, "%i", aspect_numer);
    sprintf(denom_buffer, "%i", aspect_denom);

    sprintf(min_width_buffer, "%i", min_width);
    sprintf(min_height_buffer, "%i", min_height);
    sprintf(max_width_buffer, "%i", max_width);
    sprintf(max_height_buffer, "%i", max_height);

    struct nk_context* nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    while (!(may_close && glfwWindowShouldClose(window)))
    {
        int width, height;

        glfwGetWindowSize(window, &width, &height);

        struct nk_rect area = nk_rect(0.f, 0.f, (float) width, (float) height);
        nk_window_set_bounds(nk, "main", area);

        nk_glfw3_new_frame();
        if (nk_begin(nk, "main", area, 0))
        {
            nk_layout_row_dynamic(nk, 30, 5);

            if (nk_button_label(nk, "Send")) sendNotification();
            if (nk_button_label(nk, "Edit last")) editLastNotification();
            if (nk_button_label(nk, "Retract last")) retractLastNotification();
            if (nk_button_label(nk, "Retract all")) glfwNotificationRetractAll();
            if (nk_button_label(nk, "Hide (briefly)"))
            {
                glfwHideWindow(window);

                const double time = glfwGetTime() + 3.0;
                while (glfwGetTime() < time)
                    glfwWaitEventsTimeout(1.0);

                glfwShowWindow(window);
            }

            nk_layout_row_dynamic(nk, 30, 1);

            if (glfwGetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH))
            {
                nk_label(nk, "Press H to disable mouse passthrough", NK_TEXT_CENTERED);

                if (glfwGetKey(window, GLFW_KEY_H))
                    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, false);
            }

            nk_label(nk, "Press Enter in a text field to set value", NK_TEXT_CENTERED);

            nk_flags events;
            const nk_flags flags = NK_EDIT_FIELD |
                                   NK_EDIT_SIG_ENTER |
                                   NK_EDIT_GOTO_END_ON_ACTIVATE;

            if (title_supported)
            {
                nk_layout_row_dynamic(nk, 30, 3);
                nk_label(nk, "Title", NK_TEXT_LEFT);

                events = nk_edit_string_zero_terminated(nk, flags, title_buffer,
                                                        sizeof(title_buffer),
                                                        nk_filter_default);
                if (live_edit && events & NK_EDIT_COMMITED)
                {
                    
                }
            }
            else
                nk_label(nk, "Title not supported", NK_TEXT_LEFT);
        
            if (body_supported)
            {
                nk_layout_row_dynamic(nk, 30, 3);
                nk_label(nk, "Body", NK_TEXT_LEFT);

                events = nk_edit_string_zero_terminated(nk, flags, body_buffer,
                                                        sizeof(body_buffer),
                                                        nk_filter_default);
            
                if (live_edit && events & NK_EDIT_COMMITED)
                {
                    
                }
            }
            else
                nk_label(nk, "Body not supported", NK_TEXT_LEFT);

            if (summary_supported)
            {
                nk_layout_row_dynamic(nk, 30, 3);
                nk_label(nk, "Body", NK_TEXT_LEFT);

                events = nk_edit_string_zero_terminated(nk, flags, summary_buffer,
                                                        sizeof(summary_buffer),
                                                        nk_filter_default);
            
                if (live_edit && events & NK_EDIT_COMMITED)
                {
                    
                }
            }
            else
                nk_label(nk, "Summary not supported", NK_TEXT_LEFT);
        
            bool update_ratio_limit = false;
            if (nk_checkbox_label(nk, "Aspect Ratio", &limit_aspect_ratio))
                update_ratio_limit = true;

            events = nk_edit_string_zero_terminated(nk, flags, numer_buffer,
                                                    sizeof(numer_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                aspect_numer = abs(atoi(numer_buffer));
                update_ratio_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(numer_buffer, "%i", aspect_numer);

            events = nk_edit_string_zero_terminated(nk, flags, denom_buffer,
                                                    sizeof(denom_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                aspect_denom = abs(atoi(denom_buffer));
                update_ratio_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(denom_buffer, "%i", aspect_denom);

            if (update_ratio_limit)
            {
                if (limit_aspect_ratio)
                    glfwSetWindowAspectRatio(window, aspect_numer, aspect_denom);
                else
                    glfwSetWindowAspectRatio(window, GLFW_DONT_CARE, GLFW_DONT_CARE);
            }

            bool update_size_limit = false;

            if (nk_checkbox_label(nk, "Minimum Size", &limit_min_size))
                update_size_limit = true;

            events = nk_edit_string_zero_terminated(nk, flags, min_width_buffer,
                                                    sizeof(min_width_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                min_width = abs(atoi(min_width_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(min_width_buffer, "%i", min_width);

            events = nk_edit_string_zero_terminated(nk, flags, min_height_buffer,
                                                    sizeof(min_height_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                min_height = abs(atoi(min_height_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(min_height_buffer, "%i", min_height);

            if (nk_checkbox_label(nk, "Maximum Size", &limit_max_size))
                update_size_limit = true;

            events = nk_edit_string_zero_terminated(nk, flags, max_width_buffer,
                                                    sizeof(max_width_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                max_width = abs(atoi(max_width_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(max_width_buffer, "%i", max_width);

            events = nk_edit_string_zero_terminated(nk, flags, max_height_buffer,
                                                    sizeof(max_height_buffer),
                                                    nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                max_height = abs(atoi(max_height_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(max_height_buffer, "%i", max_height);

            if (update_size_limit)
            {
                glfwSetWindowSizeLimits(window,
                                        limit_min_size ? min_width : GLFW_DONT_CARE,
                                        limit_min_size ? min_height : GLFW_DONT_CARE,
                                        limit_max_size ? max_width : GLFW_DONT_CARE,
                                        limit_max_size ? max_height : GLFW_DONT_CARE);
            }

            int fb_width, fb_height;
            glfwGetFramebufferSize(window, &fb_width, &fb_height);
            nk_label(nk, "Framebuffer Size", NK_TEXT_LEFT);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", fb_width);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", fb_height);

            float xscale, yscale;
            glfwGetWindowContentScale(window, &xscale, &yscale);
            nk_label(nk, "Content Scale", NK_TEXT_LEFT);
            nk_labelf(nk, NK_TEXT_LEFT, "%f", xscale);
            nk_labelf(nk, NK_TEXT_LEFT, "%f", yscale);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 5);
            int frame_left, frame_top, frame_right, frame_bottom;
            glfwGetWindowFrameSize(window, &frame_left, &frame_top, &frame_right, &frame_bottom);
            nk_layout_row_push(nk, 1.f / 3.f);
            nk_label(nk, "Frame Size:", NK_TEXT_LEFT);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_left);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_top);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_right);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_bottom);
            nk_layout_row_end(nk);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 2);
            float opacity = glfwGetWindowOpacity(window);
            nk_layout_row_push(nk, 1.f / 3.f);
            nk_labelf(nk, NK_TEXT_LEFT, "Opacity: %0.3f", opacity);
            nk_layout_row_push(nk, 2.f / 3.f);
            if (nk_slider_float(nk, 0.f, &opacity, 1.f, 0.001f))
                glfwSetWindowOpacity(window, opacity);
            nk_layout_row_end(nk);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 2);
            int should_close = glfwWindowShouldClose(window);
            nk_layout_row_push(nk, 1.f / 3.f);
            if (nk_checkbox_label(nk, "Should Close", &should_close))
                glfwSetWindowShouldClose(window, should_close);
            nk_layout_row_push(nk, 2.f / 3.f);
            nk_checkbox_label(nk, "May Close", &may_close);
            nk_layout_row_end(nk);

            nk_layout_row_dynamic(nk, 30, 1);
            nk_label(nk, "Attributes", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(nk, 30, width > 200 ? width / 200 : 1);

            int decorated = glfwGetWindowAttrib(window, GLFW_DECORATED);
            nk_checkbox_label(nk, "Use index", (int*) &use_index);

            int resizable = glfwGetWindowAttrib(window, GLFW_RESIZABLE);
            if (nk_checkbox_label(nk, "Resizable", &resizable))
                glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable);

            int floating = glfwGetWindowAttrib(window, GLFW_FLOATING);
            if (nk_checkbox_label(nk, "Floating", &floating))
                glfwSetWindowAttrib(window, GLFW_FLOATING, floating);

            int passthrough = glfwGetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH);
            if (nk_checkbox_label(nk, "Mouse Passthrough", &passthrough))
                glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, passthrough);

            int auto_iconify = glfwGetWindowAttrib(window, GLFW_AUTO_ICONIFY);
            if (nk_checkbox_label(nk, "Auto Iconify", &auto_iconify))
                glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, auto_iconify);

            nk_value_bool(nk, "Focused", glfwGetWindowAttrib(window, GLFW_FOCUSED));
            nk_value_bool(nk, "Hovered", glfwGetWindowAttrib(window, GLFW_HOVERED));
            nk_value_bool(nk, "Visible", glfwGetWindowAttrib(window, GLFW_VISIBLE));
            nk_value_bool(nk, "Iconified", glfwGetWindowAttrib(window, GLFW_ICONIFIED));
            nk_value_bool(nk, "Maximized", glfwGetWindowAttrib(window, GLFW_MAXIMIZED));
        }
        nk_end(nk);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(window);

        glfwWaitEvents();
    }

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


// Support:
//
// Explicit permission/authorization handling
//
// Explicit category/kind/type:
//    Action
//    
// Notification
//    Swap content
//    Standard or custom sound
//    Urgency
//    Retract
//
// Notification content
//    Title
//    Summary
//    Body
//
//    MacOS: use all 3
//    Linux: [title, summary/body] => summary: [0], body: [1]. [summary, body] => summary: [0], body: [1]. [title] => summary.

// Perhaps an opaque NotificationCategory? glfwCreateNotificationCategory(const char* name, actions, etc)
// Providing a category name should probably be optional, as GLFW can auto-generate one (only MacOS needs one), and the user only needs to specify it if they ever use other MacOS specific features. 

// So on Linux, listen for DBus signals in glfwPollEvents/glfwWaitEvents? Make no guarantees of which thread calls callbacks, so that MacOS is safely supported too?

// https://specifications.freedesktop.org/notification-spec/latest/ar01s08.html
// TODO: use the following hints: "urgency": uint8, "category": str, "desktop-entry": str, "image-data": iiibiiay, "sound-file": str, "sound-name": str, "suppress-sound": bool (and maybe "x-glfw-version": str?)
// See the server capabilities

// https://specifications.freedesktop.org/notification-spec/latest/ar01s06.html for "category" hint.
// The "category" hint should perhaps not be related to the MacOS category.



typedef struct {
    void** elements;
    int capacity;
    int count;
} _list;

typedef int (*_list_comparator)(const void* key, const void* element);

void _list_init(_list* list)
{
    list->elements = NULL;
    list->capacity = 0;
    list->count = 0;
}

void _list_deinit(_list* list)
{
    // TODO: use GLFW allocator
    free(list->elements);
    list->capacity = 0;
    list->count = 0;
}

void _list_add(_list* list, void* element)
{
    if (list->capacity == list->count)
    {
        // TODO: use GLFW allocator
        if (list->capacity == 0)
            list->capacity = 32;
        else
            list->capacity = list->capacity < 1;
            
        void** new_list = calloc(list->capacity, sizeof(void*));
        
        if (list->elements != NULL)
        {
            memcpy(new_list, list->elements, list->count);
            free(list->elements);
        }

        list->elements = new_list;
    
        list->elements[list->count++] = element;
        return;
    }
    
    for (int i = 0; i < list->capacity; ++i)
    {
        if (list->elements[i] == NULL)
        {
            list->elements[i] = element;
            break;
        }
    }
    ++list->count;
}

void* _list_remove(const _list* list, const void* key, _list_comparator comparator)
{
    for (int i = 0; i < list->capacity; ++i)
    {
        if (comparator(key, list->elements[i]))
        {
            void* element = list->elements[i];
            list->elements[i] = NULL;
            return element;
        }
    }
    return NULL;
}

void* _list_find(const _list* list, const void* key, _list_comparator comparator)
{
    for (int i = 0; i < list->capacity; ++i)
    {
        if (comparator(key, list->elements[i]))
            return list->elements[i];
    }
    return NULL;
}

static int notificationsComparator(const void* id, const void* notification)
{
    return ((_GLFWnotification*) notification)->linux.id == *((uint32_t*) id);
}

static _list retainedNotifications;

// TODO: _list_init(retainedNotifications);
// TODO: _list_deinit(retainedNotifications);

void addNotification(_GLFWnotification* notification)
{
    _list_add(&retainedNotifications, notification);
}

_GLFWnotification* removeNotification(uint32_t id)
{
    return _list_remove(&retainedNotifications, &id, notificationsComparator);
}

_GLFWnotification* findNotification(uint32_t id)
{
    return _list_find(&retainedNotifications, &id, notificationsComparator);
}


// "urgency" hint
enum org_freedesktop_Notifications_UrgencyLevel {
    LOW = 0,
    NORMAL = 1,
    CRITICAL = 2
};

enum org_freedesktop_Notifications_ClosedReason {
    EXPIRED = 1,
    DISMISSED = 2,
    CLOSED = 3,
    UNDEFINED = 4
};

struct iiibiiay_image {
    int width;
    int height;
    int rowstride;
    bool has_alpha;
    int bits_per_sample;
    int channels;
    void* data;
};

void glfwImageToIIIBIIAY(const GLFWimage* src, struct iiibiiay_image* dst)
{
    dst->width = src->width;
    dst->height = src->height;
    dst->rowstride = src->width * 4;
    dst->has_alpha = true;
    dst->bits_per_sample = 32;
    dst->channels = 4;
    dst->data = src->pixels;
}

void org_freedesktop_Notifications_GetCapabilities(const char** array, int* count);

// From spec: The "app_icon" parameter and "image-path" hint should be either an URI (file:// is the only URI schema supported right now) or a name in a freedesktop.org-compliant icon theme (not a GTK+ stock ID).

// returns the ID of the posted notification
uint32_t org_freedesktop_Notifications_Notify(const char* app_name,
                                              uint32_t replaces_id,
                                              const char* app_icon, // Not user-specified
                                              const char* summary,
                                              const char* body,
                                              const char** actions, int actions_count,
                                              const char** hints, int hints_count, // FIXME: hints signature
                                              int32_t expire_timeout); // milliseconds after display. 0: never. -1: server default




// Use for retracting a notification.
void org_freedesktop_Notifications_CloseNotification(uint32_t id) {}

void org_freedesktop_Notifications_NotificationClosed(uint32_t id, enum org_freedesktop_Notifications_ClosedReason reason) {
    
}

// TODO: What's the best design here? Pass a lambda (function pointer) to GLFW for a notification category's action, which GLFW automatically generates a string for, and adds to a dictionary in _glfw?
// MacOS actually has the exact same design, in that the delegate gets the action identifier.
// MacOS gets more than just the identifier for the action (action_key) and notification (id), though: it also gets the notification itself, including the content, userInfo, and more.
// The ID/identifier is useless by itself, so either GLFW or the client must map it to user-specified data.
// On MacOS, all this data persists between application lifetimes. On Linux, they don't, unless the client stores them on the disk. Doing that is outside the scope of Linux. Doing it on MacOS because GLFW doesn't expose it, is stupid overhead.
// Maybe pass that data to the client's callback as nullable data? So guarantee it's always present if the application didn't shut down beforehand? That way, if a client depends on notification data for their callback, and this must be available after a shutdown, they must store this on the on the disk themselves. Should provide some kind of query method to check if the platform supports it, so MacOS can report yes, and Linux reports no.
// A chat application is a good example of one 
void org_freedesktop_Notifications_ActionInvoked(uint32_t id, const char* action_key) {
    const _GLFWnotification* notification = findNotification(id);
    
    // A faulty server implementation can make a GLFW application with assertions enabled, crash by sending an invalid ID
    assert(notification != NULL);
}

// Actions can be invoked with an optional activation token. This signal is always invoked before ActionInvoked
void org_freedesktop_Notifications_ActivationToken(uint32_t id, const char* activation_token) {
    const _GLFWnotification* notification = findNotification(id);
    
    assert(notification != NULL);
    
    // Documentation for ActivationToken:
    // The ID of the notification emitting the ActionInvoked signal.
    //
    // What does this mean? Something like this? Inside app A, asks for 2FA authorization. B gets signal from server, sends a notification, user taps notification, swaps to B, finishes the action, then B automatically switches back to A with the activation_token?
}

void signalHandler(const char* signalName) {
    if (strcmp(signalName, "NotificationClosed")) {
        org_freedesktop_Notifications_NotificationClosed(-1, -1);
        return;
    }
    if (strcmp(signalName, "ActionInvoked")) {
        org_freedesktop_Notifications_ActionInvoked(-1, -1);
        return;
    }
    if (strcmp(signalName, "ActivationToken")) {
        org_freedesktop_Notifications_ActivationToken(-1, -1);
    }
}



void glfwNotificationRetractLinux(GLFWnotification* handle) {
    _GLFWnotification* notification = (_GLFWnotification*) handle;
    assert(notification->linux.id != 0);
    
    org_freedesktop_Notifications_CloseNotification(notification->linux.id);
}

void glfwNotificationRetractAllLinux() {
    for (int i = 0; i < retainedNotifications.capacity; ++i)
    {
        _GLFWnotification* notification = (_GLFWnotification*) retainedNotifications.elements[i];
        if (notification != NULL)
            org_freedesktop_Notifications_CloseNotification(notification->linux.id);
    }
}
