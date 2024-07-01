//
// Created by RobinQu on 2024/6/27.
//

#ifndef LLM_ALL_HPP
#define LLM_ALL_HPP
#include <instinct/agent/base_worker.hpp>
#include <instinct/agent/executor/agent_executor.hpp>
#include <instinct/agent/local_toolkits_worker.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_agent_executor.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_joiner.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_joiner_result_output_parser.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_joiner_task_graph_input_parser.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_planer.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_planer_agent_state_input_parser.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_planer_thought_output_parser.hpp>
#include <instinct/agent/patterns/llm_compiler/task_graph_utils.hpp>
#include <instinct/agent/patterns/openai_tool/openai_tool_agent_executor.hpp>
#include <instinct/agent/patterns/openai_tool/openai_tool_agent_planner.hpp>
#include <instinct/agent/patterns/react/agent.hpp>
#include <instinct/agent/patterns/react/react_agent_state_input_parser.hpp>
#include <instinct/agent/patterns/react/react_agent_thought_output_parser.hpp>
#include <instinct/chain/llm_chain.hpp>
#include <instinct/chain/message_chain.hpp>
#include <instinct/chat_model/base_chat_model.hpp>
#include <instinct/chat_model/ollama_chat.hpp>
#include <instinct/chat_model/openai_chat.hpp>
#include <instinct/commons/ollama_commons.hpp>
#include <instinct/commons/openai_commons.hpp>
#include <instinct/document/base_text_splitter.hpp>
#include <instinct/document/character_text_splitter.hpp>
#include <instinct/document/language_splitters.hpp>
#include <instinct/document/recursive_character_text_splitter.hpp>
#include <instinct/document/text_splitter.hpp>
#include <instinct/embedding_model/local_embedding_model.hpp>
#include <instinct/embedding_model/ollama_embedding.hpp>
#include <instinct/embedding_model/openai_embedding.hpp>
#include <instinct/input_parser/base_input_parser.hpp>
#include <instinct/input_parser/prompt_value_variant_input_parser.hpp>
#include <instinct/llm/base_llm.hpp>
#include <instinct/llm/ollama_llm.hpp>
#include <instinct/llm/openai_llm.hpp>
#include <instinct/llm_global.hpp>
#include <instinct/llm_object_factory.hpp>
#include <instinct/memory/chat_memory.hpp>
#include <instinct/memory/ephemeral_chat_memory.hpp>
#include <instinct/model/embedding_model.hpp>
#include <instinct/model/language_model.hpp>
#include <instinct/model/ranking_model.hpp>
#include <instinct/output_parser/base_output_parser.hpp>
#include <instinct/output_parser/multiline_generation_output_parser.hpp>
#include <instinct/output_parser/protobuf_message_output_parser.hpp>
#include <instinct/output_parser/string_output_parser.hpp>
#include <instinct/prompt/chat_prompt_template.hpp>
#include <instinct/prompt/example_selector.hpp>
#include <instinct/prompt/few_shot_prompt_template.hpp>
#include <instinct/prompt/message_utils.hpp>
#include <instinct/prompt/mutable_example_selector.hpp>
#include <instinct/prompt/passthrough_example_selector.hpp>
#include <instinct/prompt/plain_chat_prompt_template.hpp>
#include <instinct/prompt/plain_prompt_template.hpp>
#include <instinct/prompt/prompt_template.hpp>
#include <instinct/prompt/string_prompt_template.hpp>
#include <instinct/ranker/base_ranking_model.hpp>
#include <instinct/ranker/local_ranking_model.hpp>
#include <instinct/tokenizer/bpe_token_ranks_reader.hpp>
#include <instinct/tokenizer/gpt2_bpe_file_reader.hpp>
#include <instinct/tokenizer/regex_tokenizer.hpp>
#include <instinct/tokenizer/tiktoken_bpe_file_reader.hpp>
#include <instinct/tokenizer/tiktoken_tokenizer.hpp>
#include <instinct/tokenizer/tokenizer.hpp>

#ifdef WITH_EXPRTK
#include <instinct/toolkit/builtin/llm_math.hpp>
#endif

#include <instinct/toolkit/builtin/serp_api.hpp>
#include <instinct/toolkit/function_tool.hpp>
#include <instinct/toolkit/function_toolkit.hpp>
#include <instinct/toolkit/lambda_function_tool.hpp>
#include <instinct/toolkit/local_toolkit.hpp>
#include <instinct/toolkit/proto_message_function_tool.hpp>
#include <instinct/toolkit/search_tool.hpp>

#endif //LLM_ALL_HPP
