idf_component_register(SRCS "src/main.c" "src/web_server_task.c" "src/file_server.c" "src/mount.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "web/favicon.ico" "web/upload_script.html")
