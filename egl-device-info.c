#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>


void egl_debug_callback(EGLenum error, const char *command, EGLint messageType, EGLLabelKHR threadLabel, EGLLabelKHR objectLabel, const char* message) {
	const char* message_type_str = NULL;
	switch (messageType) {
		case EGL_DEBUG_MSG_CRITICAL_KHR: message_type_str = "CRITICAL"; break;
		case EGL_DEBUG_MSG_ERROR_KHR:    message_type_str = "ERROR";    break;
		case EGL_DEBUG_MSG_WARN_KHR:     message_type_str = "WARN";     break;
		case EGL_DEBUG_MSG_INFO_KHR:     message_type_str = "INFO";     break;
	}
	
	// Error list taken from https://registry.khronos.org/EGL/sdk/docs/man/html/eglGetError.xhtml
	const char* error_str = "unknown error";
	switch (error) {
		case EGL_SUCCESS:             error_str = "EGL_SUCCESS";             break;
		case EGL_NOT_INITIALIZED:     error_str = "EGL_NOT_INITIALIZED";     break;
		case EGL_BAD_ACCESS:          error_str = "EGL_BAD_ACCESS";          break;
		case EGL_BAD_ALLOC:           error_str = "EGL_BAD_ALLOC";           break;
		case EGL_BAD_ATTRIBUTE:       error_str = "EGL_BAD_ATTRIBUTE";       break;
		case EGL_BAD_CONTEXT:         error_str = "EGL_BAD_CONTEXT";         break;
		case EGL_BAD_CONFIG:          error_str = "EGL_BAD_CONFIG";          break;
		case EGL_BAD_CURRENT_SURFACE: error_str = "EGL_BAD_CURRENT_SURFACE"; break;
		case EGL_BAD_DISPLAY:         error_str = "EGL_BAD_DISPLAY";         break;
		case EGL_BAD_SURFACE:         error_str = "EGL_BAD_SURFACE";         break;
		case EGL_BAD_MATCH:           error_str = "EGL_BAD_MATCH";           break;
		case EGL_BAD_PARAMETER:       error_str = "EGL_BAD_PARAMETER";       break;
		case EGL_BAD_NATIVE_PIXMAP:   error_str = "EGL_BAD_NATIVE_PIXMAP";   break;
		case EGL_BAD_NATIVE_WINDOW:   error_str = "EGL_BAD_NATIVE_WINDOW";   break;
		case EGL_CONTEXT_LOST:        error_str = "EGL_CONTEXT_LOST";        break;
	}
	
	fprintf(stderr, "EGL %s %s: %s: %s\n", message_type_str, error_str, command, message);
}

void print_extensions_by_line(const char* space_separated_extension_list) {
	const char *word_start = space_separated_extension_list;
	while (*word_start != '\0') {
		const char* word_end = word_start;
		while ( !(*word_end == ' ' || *word_end == '\0') )
			word_end++;
		
		printf("%.*s\n", (int)(word_end - word_start), word_start);
		
		word_start = (*word_end == ' ') ? word_end + 1 : word_end;
	}
}

int main(int argc, char** argv) {
	// Process command line arguments
	int opengl_major_version = 4, opengl_minor_version = 5;
	
	for (int i = 1; i < argc; i++) {
		if ( strcmp(argv[i], "--opengl-version") == 0 ) {
			sscanf(argv[i+1], "%d.%d", &opengl_major_version, &opengl_minor_version);
			i++;
		} else {
			fprintf(stderr, "Unkown command line argument: %s\n", argv[i]);
			fprintf(stderr, "\n");
			fprintf(stderr, "Usage:\n");
			fprintf(stderr, "%s [--opengl-version MAJOR.MINOR]\n", argv[0]);
			return 1;
		}
	}
	
	
	// Output EGL client information
	const char* egl_client_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (egl_client_extensions == NULL) {
		fprintf(stderr, "eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS) failed, EGL_EXT_client_extensions probably not supported.\n");
		return 1;
	}
	
	printf("EGL client\n");
	printf("==========\n");
	printf("\n");
	printf("Client version: %s\n", eglQueryString(EGL_NO_DISPLAY, EGL_VERSION));
	printf("Client extensions:\n");
	printf("\n");
	print_extensions_by_line(egl_client_extensions);
	printf("\n");
	printf("\n");
	
	
	// Install EGL debug message callback if supported so we get nice error messages without having to check errors all the time
	if ( strstr(egl_client_extensions, "EGL_KHR_debug") != NULL ) {
	PFNEGLDEBUGMESSAGECONTROLKHRPROC eglDebugMessageControlKHR = (PFNEGLDEBUGMESSAGECONTROLKHRPROC)eglGetProcAddress("eglDebugMessageControlKHR");
		eglDebugMessageControlKHR(egl_debug_callback, (const EGLAttrib[]){
			// EGL_DEBUG_MSG_CRITICAL_KHR and EGL_DEBUG_MSG_ERROR_KHR are enabled by default
			EGL_DEBUG_MSG_WARN_KHR, EGL_TRUE,
			EGL_DEBUG_MSG_INFO_KHR, EGL_TRUE,
			EGL_NONE
		});
	}
	
	// List all devices
	if ( strstr(egl_client_extensions, "EGL_EXT_device_base") == NULL ) {
		fprintf(stderr, "EGL_EXT_device_base not supported, but needed for device enumeration.\n");
	} else {
		PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
		int device_count = 0;
		eglQueryDevicesEXT(0, NULL, &device_count);
		EGLDeviceEXT* devices = calloc(device_count, sizeof(devices[0]));
		eglQueryDevicesEXT(device_count, devices, &device_count);
		
		PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)eglGetProcAddress("eglQueryDeviceStringEXT");
		for (int i = 0; i < device_count; i++) {
			EGLDeviceEXT device = devices[i];
			
			// Show basic device information
			printf("EGL device %d\n", i);
			printf("============\n");
			printf("\n");
			
			const char* device_extensions = eglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
			printf("Device extensions: %s\n", device_extensions);
			if ( strstr(device_extensions, "EGL_EXT_device_drm") != NULL )
				printf("    EGL_DRM_DEVICE_FILE_EXT: %s\n", eglQueryDeviceStringEXT(device, EGL_DRM_DEVICE_FILE_EXT));
			if ( strstr(device_extensions, "EGL_EXT_device_drm_render_node") != NULL )
				printf("    EGL_DRM_RENDER_NODE_FILE_EXT: %s\n", eglQueryDeviceStringEXT(device, EGL_DRM_RENDER_NODE_FILE_EXT));
			printf("\n");
			
			// Show device display information
			EGLDisplay display = eglGetPlatformDisplay(EGL_PLATFORM_DEVICE_EXT, device, (const EGLAttrib[]){ EGL_NONE });
			if (display == EGL_NO_DISPLAY) {
				fprintf(stderr, "eglGetPlatformDisplay(EGL_PLATFORM_DEVICE_EXT, device, (const EGLAttrib[]){ EGL_NONE }) failed. Can't show display information.\n");
			} else {
				if ( eglInitialize(display, NULL, NULL) ) {
					const char* display_extensions = eglQueryString(display, EGL_EXTENSIONS);
					printf("Display version: %s\n", eglQueryString(display, EGL_VERSION));
					printf("Display vendor: %s\n", eglQueryString(display, EGL_VENDOR));
					if ( strstr(display_extensions, "EGL_MESA_query_driver") ) {
						PFNEGLGETDISPLAYDRIVERNAMEPROC eglGetDisplayDriverName = (PFNEGLGETDISPLAYDRIVERNAMEPROC)eglGetProcAddress("eglGetDisplayDriverName");
						printf("Display driver name: %s (from EGL_MESA_query_driver)\n", eglGetDisplayDriverName(display));
					}
					printf("Display APIs: %s\n", eglQueryString(display, EGL_CLIENT_APIS));
					printf("Display extensions:\n");
					printf("\n");
					print_extensions_by_line(display_extensions);
					printf("\n");
					
					if ( eglBindAPI(EGL_OPENGL_API) ) {
						if ( strstr(display_extensions, "EGL_KHR_no_config_context") == NULL || strstr(display_extensions, "EGL_KHR_surfaceless_context") == NULL ) {
							fprintf(stderr, "EGL_KHR_no_config_context and EGL_KHR_surfaceless_context are not supported but needed to create an OpenGL context. Can't show OpenGL information.\n");
						} else {
							// EGL_KHR_no_config_context allows us to skip the entire config thing and just pass EGL_NO_CONFIG_KHR.
							EGLContext context = eglCreateContext(display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, (EGLint const[]){
								EGL_CONTEXT_MAJOR_VERSION, opengl_major_version,
								EGL_CONTEXT_MINOR_VERSION, opengl_minor_version,
								EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
								// Those two don't work on EGL v1.4? (with OpenGL renderer "Mesa DRI Intel(R) UHD Graphics 620 (KBL GT2)")
								//EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
								//EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
								EGL_NONE
							});
							
							// With EGL_KHR_surfaceless_context we can pass EGL_NO_SURFACE as the EGL draw and read surfaces. EGL_KHR_create_context also allows that for OpenGL.
							if ( context != EGL_NO_CONTEXT && eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) ) {
								printf("OpenGL version: %s\n", glGetString(GL_VERSION));
								printf("OpenGL shading language version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
								printf("OpenGL vendor: %s\n", glGetString(GL_VENDOR));
								printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
								printf("OpenGL extensions:\n");
								printf("\n");
								GLint gl_extension_count = 0;
								glGetIntegerv(GL_NUM_EXTENSIONS, &gl_extension_count);
								for (int i = 0; i < gl_extension_count; i++)
									printf("%s\n", glGetStringi(GL_EXTENSIONS, i));
							}
						}
					}
					
					eglTerminate(display);
				}
			}
			
			printf("\n");
			printf("\n");
		}
		
		free(devices);
	}
	
	return 0;
}
