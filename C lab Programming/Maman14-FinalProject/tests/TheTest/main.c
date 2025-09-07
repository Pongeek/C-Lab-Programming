#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include "../../headers/lexer.h"
#include "../../headers/preprocessor.h"
#include "../../headers/parser.h"
#include "../../headers/semantic_analyzer.h"
#include "../../headers/code_generator.h"

/*This structure allows for batch processing of multiple assembly files, with each successful compilation resulting in its own output folder containing the generated files.
If any stage fails for a file, it moves on to the next file without generating output for the failed one.*/

int create_directory(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path,0700) == -1) {
            perror("Error creating directory");
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    Lexer lexer_preprocess;
    Lexer lexer_postprocess;
    Preprocessor preprocessor;
    TranslationUnit unit;
    SemanticAnalyzer analyzer;
    CodeGenerator generator;
    int i;

    if (argc < 2) {
        printf("Usage: %s <file1.as> [file2.as ...]\n", argv[0]);
        return 1;
    }

for (i = 1; i < argc; i++) {
    char output_dir[256];
    char output_file[256];
    char *dot;
    char *base_name;

    printf("Processing file: %s\n", argv[i]);

    /* preprocess lexer */
    if (lexer_initialize_from_file(&lexer_preprocess, argv[i]) == 1) {
        printf("Lexical analysis (pre-process) started...\n");
        lexer_analyze(&lexer_preprocess);
        error_handler_report_errors(&lexer_preprocess.error_handler);

        if (lexer_preprocess.error_handler.error_list == NULL) {
            /* preprocessor */
            printf("Preprocessing started...\n");
            preprocessor_initialize(&preprocessor, lexer_preprocess, argv[i]);
            preprocessor_process(&preprocessor, lexer_preprocess.source_code);
            error_handler_report_errors(&preprocessor.error_handler);

            if (preprocessor.error_handler.error_list == NULL) {
                /* postprocess lexer */
                printf("Lexical analysis (post-process) started...\n");
                lexer_initialize_from_string(&lexer_postprocess, preprocessor.error_handler.file_path, preprocessor.processed_source);
                lexer_analyze(&lexer_postprocess);
                error_handler_report_errors(&lexer_postprocess.error_handler);

                if (lexer_postprocess.error_handler.error_list == NULL) {
                    /* parser */
                    printf("Parsing started...\n");
                    parser_initialize_translation_unit(&unit, lexer_postprocess);
                    parse_translation_unit_content(&unit);
                    error_handler_report_errors(&unit.error_handler);

                    if (unit.error_handler.error_list == NULL) {
                        /* Analyzer */
                        printf("Semantic analysis started...\n");
                        semantic_analyzer_initialize(&analyzer, &unit, lexer_postprocess);
                        semantic_analyzer_analyze_translation_unit(&analyzer, &unit);
                        error_handler_report_errors(&analyzer.error_handler);

                        if (analyzer.error_handler.error_list == NULL) {
                            /* Create output directory name */
                            base_name = strrchr(argv[i], '/');
                            if (base_name == NULL) {
                                base_name = argv[i];
                            } else {
                                base_name++;
                            }
                            sprintf(output_dir, "%s_output", base_name);
                            dot = strrchr(output_dir, '.');
                            if (dot) *dot = '\0';

                            /* Create output directory */
                            if (!create_directory(output_dir)) {
                                printf("Failed to create output directory for %s\n", argv[i]);
                                continue;
                            }

                            /* Code generator */
                            printf("Code generation started...\n");
                            code_generator_initialize(&generator, lexer_postprocess);
                            code_generator_update_labels(&generator, &unit);
                            generate_entry_file_string(&generator, &analyzer, &unit);

                            /* Create output file path */
                            sprintf(output_file, "%s/%s", output_dir, base_name);
                            dot = strrchr(output_file, '.');
                            if (dot) *dot = '\0';
                            strcat(output_file, ".ob");

                            output_generate(&generator, &analyzer, &unit, output_file);
                            error_handler_report_errors(&generator.error_handler);

                            code_generator_free(&generator);
                        }

                        semantic_analyzer_free(&analyzer);
                    }

                    parser_free_translation_unit(&unit);
                }

                lexer_free(&lexer_postprocess);
            }

            preprocessor_free(&preprocessor);
        }
    }

    lexer_free(&lexer_preprocess);
    printf("Finished processing file: %s\n\n", argv[i]);
}

    return 0;
}
