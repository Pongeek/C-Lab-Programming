cmake_minimum_required(VERSION 3.28)
project(CProject C)

set(CMAKE_C_STANDARD 90)

add_executable(CProject
        headers/semantic_analyzer.h
        headers/char_util.h
        headers/code_generator.h
        headers/error_handler.h
        headers/lexer.h
        headers/nodes.h
        headers/parser.h
        headers/preprocessor.h
        headers/safe_allocations.h
        headers/string_util.h
        headers/token.h
        source/semantic_analyzer.c
        source/code_generator.c
        source/error_handler.c
        source/lexer.c
        source/parser.c
        source/preprocessor.c
        source/safe_allocations.c
        tests/parser/parse_data_directive_guidance/parse_data_directive_guidance_test.c
        tests/parser/parser_parse_guidance_list/parser_parse_guidance_list.c
        utils/char_util.c
        tests/code_generator/output_generate_test/output_generate_test.c
        utils/string_util.c
        tests/semantic_analyzer/analyze_directive_guidance/analyze_directive_guidance.c
        tests/semantic_analyzer/analyze_label/semantic_analyzer_analyze_label.c
        tests/semantic_analyzer/analyze_instruction/semantic_analyzer_analyze_instruction.c
        tests/semantic_analyzer/analyze_duplicate_identifiers/analyze_duplicate_identifiers.c
        tests/semantic_analyzer/analyze_translation_unit/analyze_translation_unit.c
        tests/lexer/tokenize_string_test/peek_string_test.c
        tests/lexer/tokenize_seperator_test/tokenize_seperator_test.c
        tests/lexer/tokenize_number_test/tokenize_number_test.c
        tests/lexer/tokenize_nonOp_identifiers_test/tokenize_nonOp_identifiers_test.c
        tests/lexer/tokenize_registers_test/tokenize_registers_test.c
        tests/lexer/lexer_analyze_test/lexer_analyze_test.c
        tests/preprocess/preprocessor_process_test/preprocessor_process_test.c
        tests/preprocess/create_macro_list_test/create_macro_list_test.c
        tests/parser/parser_parse_instruction/parser_parse_instruction_test.c
        tests/parser/parser_parse_instruction_list/parser_parse_instruction_list_test.c
        tests/parser/parse_translation_unit_content/parse_translation_unit_content.c
        tests/code_generator/generate_object_and_external_files/generate_object_and_external_files.c
        tests/code_generator/generate_entry_file_string/generate_entry_file_string.c
        "tests/THE TEST/main.c"

)
