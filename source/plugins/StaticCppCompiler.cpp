#include "StaticCppCompiler.h"

StaticCppCompilerError::StaticCppCompilerError(const char *msg)
    : message(msg) {}

const char *StaticCppCompilerError::what() const noexcept {
  return message.c_str();
}

void* StaticCppCompiler::loadLibrary(const char *path, int flags) {
    void *dynamicLibraryHandle = dlopen(path, flags);
    const char *error = dlerror();
    if (error != nullptr) {
      throw StaticCppCompilerError(error);
    }
    return dynamicLibraryHandle;
  }
