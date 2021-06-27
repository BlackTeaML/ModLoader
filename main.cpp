#include <vector>
#include <fstream>
#include <jni.h>
#include <sstream>
#include <unistd.h>
#include <filesystem>
#include <dlfcn.h>

bool isMod(std::string const& path) {
    std::string s;
    std::stringstream ss(path);
    while(std::getline(ss, s, '.'));
    return s == "so";
}

JNIEXPORT jint JNI_OnLoad(JavaVM*, void*) {
    pid_t pid = getpid();
    char path[64] = { 0 };
    sprintf(path, "/proc/%d/cmdline", pid);
    FILE *cmdline = fopen(path, "r");
    char application_id[64] = { 0 };
    fread(application_id, sizeof(application_id), 1, cmdline);
    std::string modules_path = "/sdcard/BlackTea/" + std::string(application_id) + "/modules";
    std::string game_path = "/data/data/" + std::string(application_id) + "/files";
    fclose(cmdline);

    for(const auto& file : std::filesystem::directory_iterator(game_path)) {
        // нуахули
        system(("rm " + game_path + '/' + file.path().filename().string()).c_str());
    }

    if (!std::filesystem::is_directory(modules_path) || !std::filesystem::exists(modules_path)) 
        std::filesystem::create_directories(modules_path);

    for(const auto& file : std::filesystem::directory_iterator(modules_path)) {
        if(isMod(file.path().filename().string())) {
            std::filesystem::copy_file(modules_path + '/' + file.path().filename().string(), game_path + '/' + file.path().filename().string());
            void* handle = dlopen((game_path + '/' + file.path().filename().string()).c_str(), RTLD_LAZY);
            void (*TeaInit)();

            *(void**)(&TeaInit) = dlsym(handle, "TeaInit");
            (*TeaInit)();
        }
    }

    return JNI_VERSION_1_4;
}
