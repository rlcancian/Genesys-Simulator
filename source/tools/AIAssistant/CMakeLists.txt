add_library(genesys_ai_provider STATIC
    AIConversationService.cpp
    AnthropicProviderClientImpl.cpp
    HttpProviderClientBase.cpp
    LocalProviderClientImpl.cpp
    OpenAIProviderClientImpl.cpp
)

target_include_directories(genesys_ai_provider PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_compile_features(genesys_ai_provider PUBLIC cxx_std_23)
