/*
 * File:   StaticCppCompiler.h
 * Author: Otávio Augusto S. Jatobá
 *
 * Created on June 28 2026, 18:50
 */

#pragma once

#include <dlfcn.h>
#include <exception>
#include <filesystem>
#include <string>

class StaticCppCompilerError : public std::exception {
private:
  std::string message;

public:
  StaticCppCompilerError(const char *msg);

  const char *what() const noexcept override;
};

class StaticCppCompiler {
private:
  static constexpr const char *defaultCompiler = "g++";

public:
  static void compileToExecutable(const char *targetPath,
                                  const char *compiler = defaultCompiler,
                                  const char *flags = "") {

    if (!std::filesystem::exists(targetPath)) {
      throw StaticCppCompilerError("");
    }
  }

  static void compileToDynamicLibrary(const char *targetPath,
                                      const char *compiler = defaultCompiler,
                                      const char *flags = "") {

    if (!std::filesystem::exists(targetPath)) {
      throw StaticCppCompilerError("");
    }
  }

  static void compileToStaticLibrary(const char *targetPath,
                                     const char *compiler = defaultCompiler,
                                     const char *flags = "");

  /*!
   * \throws StaticCppCompilerError
   * \return The dynamic library handle.
   */
  static void *loadLibrary(const char *path, int flags = RTLD_LAZY);
};
