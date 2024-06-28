cmake_minimum_required(VERSION 3.26)
project(instinct_llm)

set(LIBRARY_TARGET_NAME llm)

set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/llm_global.hpp
        include/instinct/chain/llm_chain.hpp
        include/instinct/llm/base_llm.hpp
        include/instinct/chat_model/base_chat_model.hpp
        include/instinct/model/language_model.hpp
        include/instinct/commons/ollama_commons.hpp
        include/instinct/llm/ollama_llm.hpp
        include/instinct/chat_model/ollama_chat.hpp
        include/instinct/embedding_model/ollama_embedding.hpp
        include/instinct/prompt/chat_prompt_template.hpp
        include/instinct/prompt/string_prompt_template.hpp
        include/instinct/prompt/plain_prompt_template.hpp
        include/instinct/prompt/plain_chat_prompt_template.hpp
        include/instinct/prompt/few_shot_prompt_template.hpp
        include/instinct/prompt/example_selector.hpp
        include/instinct/prompt/mutable_example_selector.hpp
        include/instinct/prompt/passthrough_example_selector.hpp
        include/instinct/prompt/length_based_example_selector.h
        include/instinct/output_parser/base_output_parser.hpp
        include/instinct/prompt/message_utils.hpp
        include/instinct/tokenizer/tokenizer.hpp
        include/instinct/tokenizer/regex_tokenizer.hpp
        include/instinct/tokenizer/tiktoken_tokenizer.hpp
        include/instinct/tokenizer/bpe_token_ranks_reader.hpp
        include/instinct/tokenizer/gpt2_bpe_file_reader.hpp
        include/instinct/tokenizer/tiktoken_bpe_file_reader.hpp
        include/instinct/memory/ephemeral_chat_memory.hpp
        include/instinct/output_parser/base_output_parser.hpp
        include/instinct/memory/chat_memory.hpp
        include/instinct/chat_model/openai_chat.hpp
        include/instinct/commons/openai_commons.hpp
        include/instinct/embedding_model/openai_embedding.hpp
        include/instinct/llm_test_global.hpp
        include/instinct/llm/openai_llm.hpp
        include/instinct/prompt/prompt_template.hpp
        include/instinct/input_parser/base_input_parser.hpp
        include/instinct/chain/message_chain.hpp
        include/instinct/output_parser/protobuf_message_output_parser.hpp
        include/instinct/output_parser/multiline_generation_output_parser.hpp
        include/instinct/input_parser/prompt_value_variant_input_parser.hpp
        include/instinct/output_parser/string_output_parser.hpp
        include/instinct/toolkit/function_tool.hpp
        include/instinct/toolkit/lambda_function_tool.hpp
        include/instinct/toolkit/function_toolkit.hpp
        include/instinct/toolkit/local_toolkit.hpp
        include/instinct/agent/patterns/react/react_agent_thought_output_parser.hpp
        include/instinct/agent/patterns/react/react_agent_state_input_parser.hpp
        include/instinct/agent/patterns/react/agent.hpp
        include/instinct/agent/executor/agent_executor.hpp
        include/instinct/agent/base_worker.hpp
        include/instinct/toolkit/proto_message_function_tool.hpp
        include/instinct/toolkit/search_tool.hpp
        include/instinct/toolkit/builtin/serp_api.hpp
        include/instinct/agent/patterns/openai_tool/openai_tool_agent_executor.hpp
        include/instinct/agent/patterns/llm_compiler/llm_compiler_agent_executor.hpp
        include/instinct/agent/patterns/llm_compiler/llm_compiler_planer.hpp
        include/instinct/agent/patterns/llm_compiler/llm_compiler_joiner.hpp
        include/instinct/agent/patterns/openai_tool/openai_tool_agent_planner.hpp
        include/instinct/agent/patterns/llm_compiler/llm_compiler_planer_agent_state_input_parser.hpp
        include/instinct/agent/patterns/llm_compiler/llm_compiler_planer_thought_output_parser.hpp
        include/instinct/agent/patterns/llm_compiler/task_graph_utils.hpp
        include/instinct/agent/patterns/llm_compiler/llm_compiler_joiner_task_graph_input_parser.hpp
        include/instinct/agent/patterns/llm_compiler/llm_compiler_joiner_result_output_parser.hpp
        include/instinct/agent/local_toolkits_worker.hpp
        include/instinct/llm_object_factory.hpp
        include/instinct/ranker/base_ranking_model.hpp
        include/instinct/model/ranking_model.hpp
        include/instinct/ranker/local_ranking_model.hpp
        include/instinct/embedding_model/local_embedding_model.hpp
)

if (WITH_EXPRTK)
    list(APPEND ${LIBRARY_TARGET_NAME}_HDR include/instinct/toolkit/builtin/llm_math.hpp)
endif()

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
        include/instinct/llm_all.hpp
)

add_library(instinct::llm
        ALIAS ${LIBRARY_TARGET_NAME})

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::core
        instinct::transformer
)

if (WITH_EXPRTK)
    target_compile_definitions(${LIBRARY_TARGET_NAME} INTERFACE WITH_EXPRTK=1)
    target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
            exprtk::exprtk
    )
endif ()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")