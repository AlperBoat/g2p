#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define CLEAN_AND_COPY_TO_VAL_ARRAY(dest_array, source_segment_start, source_segment_length)                                    \
    do{                                                                                                                         \
        const char *macro_src_ptr = (source_segment_start);                                                                     \
        int macro_src_len = (source_segment_length);                                                                            \
        const char *macro_current_ptr = macro_src_ptr;                                                                          \
        const char *macro_segment_end_ptr = macro_src_ptr + macro_src_len;                                                      \
        while (macro_current_ptr < macro_segment_end_ptr && isspace((unsigned char)*macro_current_ptr))                         \
        {                                                                                                                       \
            macro_current_ptr++;                                                                                                \
        }                                                                                                                       \
        const char *macro_actual_content_end_ptr = macro_segment_end_ptr;                                                       \
        while (macro_actual_content_end_ptr > macro_current_ptr && isspace((unsigned char)*(macro_actual_content_end_ptr - 1))) \
        {                                                                                                                       \
            macro_actual_content_end_ptr--;                                                                                     \
        }                                                                                                                       \
        int macro_cleaned_len = macro_actual_content_end_ptr - macro_current_ptr;                                               \
        if (macro_cleaned_len < 0)                                                                                              \
            macro_cleaned_len = 0;                                                                                              \
        int macro_len_to_copy = (macro_cleaned_len < 15) ? macro_cleaned_len : 15;                                              \
        if (macro_len_to_copy > 0)                                                                                              \
        {                                                                                                                       \
            strncpy((dest_array), macro_current_ptr, macro_len_to_copy);                                                        \
        }                                                                                                                       \
        (dest_array)[macro_len_to_copy] = '\0';                                                                                 \
    } while (0)

struct StringGroupNode
{
    char *groupValue;
    struct StringGroupNode *next;
};

struct grammer
{
    char val[16];
    struct head *Head;
    struct grammer *next;
};

struct head
{
    char val[16];
    struct head *nextHead;
    struct grammer *gr;
};

void remove_spaces(char *s)
{
    char *d = s;
    char *current_s = s;
    do
    {
        while (*d == ' ')
        {
            ++d;
        }
    } while ((*current_s++ = *d++));
}

void fileNameObtain(char fileName[64])
{
    printf("Enter File Name: ");
    if (scanf("%63s", fileName) == 1)
    {
        if (strlen(fileName) < 64 - strlen(".txt") - 1)
        {
            strcat(fileName, ".txt");
        }
        else
        {
            printf("Error: Entered file name is too long to append .txt safely.\n");
            fileName[0] = '\0';
        }
    }
    else
    {
        printf("Error reading input.\n");
        if (fileName != NULL && 64 > 0)
        {
            fileName[0] = '\0';
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
}

int ftprVaild(char fileName[64])
{
    FILE *local_ftpr = fopen(fileName, "r");
    if (local_ftpr != NULL)
    {
        fclose(local_ftpr);
        return 0;
    }

    while (1)
    {
        int selection = -1;
        printf("\nThe file named '%s' was not found. Do you want to create this file? (Yes: 1, No: 0): ", fileName);

        if (scanf("%d", &selection) == 1)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;

            switch (selection)
            {
            case 0:
                printf("\nExiting program as file is required.\n");
                return -1;
            case 1:
                printf("\nCreating the file named '%s'.\n", fileName);
                FILE *temp_ftpr = fopen(fileName, "w");
                if (temp_ftpr == NULL)
                {
                    perror("Error: Could not create the file");
                    return -1;
                }
                printf("\nThe file '%s' has been created. Please enter the grammar into this file and then rerun the program.\nExiting program...\n", fileName);
                fclose(temp_ftpr);
                return -1;
            default:
                printf("\nInvalid input! Please enter 0 or 1.\n");
            }
        }
        else
        {
            printf("\nInvalid input format! Please enter a number (0 or 1).\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
        }
    }
}

void freeStringGroupList(struct StringGroupNode *node)
{
    struct StringGroupNode *current = node;
    while (current != NULL)
    {
        struct StringGroupNode *next_node = current->next;
        if (current->groupValue)
        {
            free(current->groupValue);
        }
        free(current);
        current = next_node;
    }
}

void freeGrammarList(struct head *headNode)
{
    struct head *currentHead = headNode;
    while (currentHead != NULL)
    {
        struct head *nextHeadTemp = currentHead->nextHead;
        struct grammer *currentGrammar = currentHead->gr;
        while (currentGrammar != NULL)
        {
            struct grammer *nextGrammarTemp = currentGrammar->next;
            free(currentGrammar);
            currentGrammar = nextGrammarTemp;
        }
        free(currentHead);
        currentHead = nextHeadTemp;
    }
}

struct head *parseGrammer(const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
    {
        fprintf(stderr, "Error: Filename is NULL or empty.\n");
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening grammar file");
        fprintf(stderr, "Filename was: %s\n", filename);
        return NULL;
    }

    char line_buffer[512];
    struct head *master_list_head = NULL;
    struct head *master_list_tail = NULL;
    int memory_error_flag = 0;

    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL)
    {
        if (memory_error_flag)
        {
            break;
        }

        line_buffer[strcspn(line_buffer, "\n\r")] = '\0';

        const char *production_arrow = strchr(line_buffer, '>');
        if (production_arrow == NULL)
        {
            int is_effectively_empty = 1;
            for (int i = 0; line_buffer[i] != '\0'; ++i)
            {
                if (!isspace((unsigned char)line_buffer[i]))
                {
                    is_effectively_empty = 0;
                    break;
                }
            }
            if (!is_effectively_empty)
            {
                fprintf(stderr, "Warning: Skipping line (no '>' separator): %s\n", line_buffer);
            }
            continue;
        }

        struct head *current_parsed_head = (struct head *)malloc(sizeof(struct head));
        if (current_parsed_head == NULL)
        {
            perror("Error: Malloc failed for head structure");
            memory_error_flag = 1;
            break;
        }
        current_parsed_head->val[0] = '\0';
        current_parsed_head->gr = NULL;
        current_parsed_head->nextHead = NULL;

        int head_val_src_len = production_arrow - line_buffer;
        CLEAN_AND_COPY_TO_VAL_ARRAY(current_parsed_head->val, line_buffer, head_val_src_len);

        const char *productions_full_string = production_arrow + 1;
        const char *current_production_segment_start = productions_full_string;
        struct grammer *grammar_list_for_this_head = NULL;
        struct grammer *grammar_list_tail_for_this_head = NULL;
        int inner_loop_mem_error = 0;

        while (*current_production_segment_start != '\0')
        {
            const char *next_pipe_separator = strchr(current_production_segment_start, '|');
            const char *current_production_segment_end;
            int current_production_len;

            if (next_pipe_separator == NULL)
            {
                current_production_segment_end = current_production_segment_start + strlen(current_production_segment_start);
            }
            else
            {
                current_production_segment_end = next_pipe_separator;
            }
            current_production_len = current_production_segment_end - current_production_segment_start;

            struct grammer *new_gram_node = (struct grammer *)malloc(sizeof(struct grammer));
            if (new_gram_node == NULL)
            {
                perror("Error: Malloc failed for grammer structure");
                inner_loop_mem_error = 1;
                break;
            }
            new_gram_node->val[0] = '\0';
            new_gram_node->next = NULL;
            new_gram_node->Head = current_parsed_head;

            CLEAN_AND_COPY_TO_VAL_ARRAY(new_gram_node->val, current_production_segment_start, current_production_len);

            if (grammar_list_for_this_head == NULL)
            {
                grammar_list_for_this_head = new_gram_node;
                grammar_list_tail_for_this_head = new_gram_node;
            }
            else
            {
                grammar_list_tail_for_this_head->next = new_gram_node;
                grammar_list_tail_for_this_head = new_gram_node;
            }

            if (next_pipe_separator == NULL)
            {
                break;
            }
            current_production_segment_start = next_pipe_separator + 1;
        }

        if (inner_loop_mem_error)
        {
            struct grammer *g_to_free = grammar_list_for_this_head;
            while (g_to_free != NULL)
            {
                struct grammer *next_g = g_to_free->next;
                free(g_to_free);
                g_to_free = next_g;
            }
            free(current_parsed_head);
            memory_error_flag = 1;
            break;
        }

        current_parsed_head->gr = grammar_list_for_this_head;

        if (master_list_head == NULL)
        {
            master_list_head = current_parsed_head;
            master_list_tail = current_parsed_head;
        }
        else
        {
            master_list_tail->nextHead = current_parsed_head;
            master_list_tail = current_parsed_head;
        }
    }

    fclose(file);

    if (memory_error_flag)
    {
        freeGrammarList(master_list_head);
        return NULL;
    }

    return master_list_head;
}

void printGrammarList(struct head *absHead)
{
    struct head *current_head_node = absHead;
    int rule_counter = 1;

    if (current_head_node == NULL)
    {
        printf("The grammar list is empty or NULL.\n");
        return;
    }

    printf("========== Grammar List ==========\n");
    while (current_head_node != NULL)
    {
        printf("Rule #%d:\n", rule_counter);
        printf(" Head : \"%s\"\n", current_head_node->val);

        if (current_head_node->gr != NULL)
        {
            struct grammer *current_grammer_rule = current_head_node->gr;
            printf(" Production : \"%s\"\n", current_grammer_rule->val);
            current_grammer_rule = current_grammer_rule->next;

            while (current_grammer_rule != NULL)
            {
                printf(" | \"%s\"\n", current_grammer_rule->val);
                current_grammer_rule = current_grammer_rule->next;
            }
        }
        else
        {
            printf(" Production : [No grammar rule attached]\n");
        }

        if (current_head_node->nextHead != NULL)
        {
            printf("------------------------------\n");
        }

        current_head_node = current_head_node->nextHead;
        rule_counter++;
    }
    printf("==============================\n");
}

struct head *findFirstHeadMatchingAnyGroup(struct StringGroupNode *groupsList, struct head *grammarHeadsList)
{
    if (groupsList == NULL || grammarHeadsList == NULL)
    {
        return NULL;
    }

    struct StringGroupNode *currentGroup = groupsList;
    while (currentGroup != NULL)
    {
        if (currentGroup->groupValue != NULL)
        {
            const char *searchTerm = currentGroup->groupValue;
            struct head *currentHead = grammarHeadsList;
            while (currentHead != NULL)
            {
                if (strcmp(currentHead->val, searchTerm) == 0)
                {
                    return currentHead;
                }
                currentHead = currentHead->nextHead;
            }
        }
        currentGroup = currentGroup->next;
    }
    return NULL;
}

struct head *searchHeadByValue(struct head *listHead, const char *searchTerm)
{
    if (searchTerm == NULL || listHead == NULL)
    {
        return NULL;
    }

    struct head *currentNode = listHead;
    while (currentNode != NULL)
    {
        if (strcmp(currentNode->val, searchTerm) == 0)
        {
            return currentNode;
        }
        currentNode = currentNode->nextHead;
    }
    return NULL;
}

static struct StringGroupNode *groupStringBySpaces(const char *inputString)
{
    if (inputString == NULL)
    {
        return NULL;
    }
    struct StringGroupNode *list_head = NULL;
    struct StringGroupNode *list_tail = NULL;
    int memory_error_occurred = 0;
    const char *ptr = inputString;

    while (*ptr != '\0' && !memory_error_occurred)
    {
        while (*ptr != '\0' && isspace((unsigned char)*ptr))
        {
            ptr++;
        }
        if (*ptr == '\0')
        {
            break;
        }
        const char *word_start = ptr;
        while (*ptr != '\0' && !isspace((unsigned char)*ptr))
        {
            ptr++;
        }
        int word_len = ptr - word_start;
        if (word_len > 0)
        {
            char *word_copy = (char *)malloc(word_len + 1);
            if (word_copy == NULL)
            {
                perror("groupStringBySpaces: Malloc failed for word_copy");
                memory_error_occurred = 1;
                break;
            }
            strncpy(word_copy, word_start, word_len);
            word_copy[word_len] = '\0';
            struct StringGroupNode *new_node = (struct StringGroupNode *)malloc(sizeof(struct StringGroupNode));
            if (new_node == NULL)
            {
                perror("groupStringBySpaces: Malloc failed for new_node");
                free(word_copy);
                memory_error_occurred = 1;
                break;
            }
            new_node->groupValue = word_copy;
            new_node->next = NULL;
            if (list_head == NULL)
            {
                list_head = new_node;
                list_tail = new_node;
            }
            else
            {
                list_tail->next = new_node;
                list_tail = new_node;
            }
        }
    }
    if (memory_error_occurred)
    {
        freeStringGroupList(list_head);
        return NULL;
    }
    return list_head;
}

static struct head *findHeadByValue(struct head *listHead, const char *searchTerm)
{
    if (searchTerm == NULL || listHead == NULL)
    {
        return NULL;
    }

    struct head *currentNode = listHead;
    while (currentNode != NULL)
    {
        if (strcmp(currentNode->val, searchTerm) == 0)
        {
            return currentNode;
        }
        currentNode = currentNode->nextHead;
    }
    return NULL;
}

static int tryParseSymbolRecursive(const char *symbolToParse, struct StringGroupNode **currentTokenNodePtr, struct head *fullGrammar)
{
    if (symbolToParse == NULL)
    {
        fprintf(stderr, "tryParseSymbolRecursive: symbolToParse is NULL\n");
        return 0;
    }
    if (currentTokenNodePtr == NULL)
    {
        fprintf(stderr, "tryParseSymbolRecursive: currentTokenNodePtr is NULL (the pointer to pointer)\n");
        return 0;
    }

    struct head *ruleForSymbol = findHeadByValue(fullGrammar, symbolToParse);

    if (ruleForSymbol != NULL)
    {
        struct StringGroupNode *tokensBeforeThisRuleAttempt = *currentTokenNodePtr;
        struct grammer *currentProduction = ruleForSymbol->gr;

        while (currentProduction != NULL)
        {
            *currentTokenNodePtr = tokensBeforeThisRuleAttempt;

            if (*(currentProduction->val) == '\0')
            {
                return 1;
            }

            struct StringGroupNode *productionRuleTokens = groupStringBySpaces(currentProduction->val);
            if (productionRuleTokens == NULL && *(currentProduction->val) != '\0')
            {
                currentProduction = currentProduction->next;
                continue;
            }

            struct StringGroupNode *originalProdRuleTokensHead = productionRuleTokens;
            int allSymbolsInProductionMatched = 1;

            if (productionRuleTokens == NULL)
            {
                allSymbolsInProductionMatched = 1;
            }
            else
            {
                struct StringGroupNode *currentSymbolInProduction = productionRuleTokens;
                while (currentSymbolInProduction != NULL)
                {
                    if (currentSymbolInProduction->groupValue == NULL)
                    {
                        allSymbolsInProductionMatched = 0;
                        break;
                    }
                    if (!tryParseSymbolRecursive(currentSymbolInProduction->groupValue, currentTokenNodePtr, fullGrammar))
                    {
                        allSymbolsInProductionMatched = 0;
                        break;
                    }
                    currentSymbolInProduction = currentSymbolInProduction->next;
                }
            }

            freeStringGroupList(originalProdRuleTokensHead);

            if (allSymbolsInProductionMatched)
            {
                return 1;
            }
            currentProduction = currentProduction->next;
        }

        *currentTokenNodePtr = tokensBeforeThisRuleAttempt;
        return 0;
    }
    else
    {
        if (*currentTokenNodePtr != NULL &&
            (*currentTokenNodePtr)->groupValue != NULL &&
            strcmp((*currentTokenNodePtr)->groupValue, symbolToParse) == 0)
        {
            *currentTokenNodePtr = (*currentTokenNodePtr)->next;
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void validateStatementsFromFile(struct head *grammarRules, const char *statementFileName)
{
    if (grammarRules == NULL)
    {
        fprintf(stderr, "Error: Grammar rules are NULL. Cannot validate statements.\n");
        return;
    }
    if (statementFileName == NULL || statementFileName[0] == '\0')
    {
        fprintf(stderr, "Error: Statement filename is NULL or empty.\n");
        return;
    }

    const char *startSymbol = grammarRules->val;
    if (*startSymbol == '\0')
    {
        fprintf(stderr, "Error: Could not determine a valid start symbol (first head's value is empty).\n");
        return;
    }

    FILE *statementsFile = fopen(statementFileName, "r");
    if (statementsFile == NULL)
    {
        perror("Error opening statement file");
        fprintf(stderr, "Filename: %s\n", statementFileName);
        return;
    }

    char line_buffer[1024];
    int line_number = 0;

    printf("Validating statements from file: %s\n", statementFileName);
    printf("Using start symbol: %s\n", startSymbol);
    printf("--------------------------------------------------\n");

    while (fgets(line_buffer, sizeof(line_buffer), statementsFile) != NULL)
    {
        line_number++;
        line_buffer[strcspn(line_buffer, "\n\r")] = '\0';

        char *trimmed_line_ptr = line_buffer;
        while (isspace((unsigned char)*trimmed_line_ptr))
        {
            trimmed_line_ptr++;
        }
        size_t len = strlen(trimmed_line_ptr);
        while (len > 0 && isspace((unsigned char)trimmed_line_ptr[len - 1]))
        {
            trimmed_line_ptr[--len] = '\0';
        }

        if (*trimmed_line_ptr == '\0')
        {
            printf("Line %d: \"\" -> INVALID (Empty line after trimming)\n", line_number);
            continue;
        }

        struct StringGroupNode *statement_tokens_head = groupStringBySpaces(trimmed_line_ptr);
        // Inside validateStatementsFromFile, after groupStringBySpaces
        printf("Debug: Tokens for input '%s': ", trimmed_line_ptr);
        struct StringGroupNode *temp_debug_tokens = statement_tokens_head;
        while (temp_debug_tokens != NULL)
        {
            printf("'%s' ", temp_debug_tokens->groupValue); // Assuming 'value' field holds token string
            temp_debug_tokens = temp_debug_tokens->next;
        }
        printf("\n");
        if (statement_tokens_head == NULL && *trimmed_line_ptr != '\0')
        {
            fprintf(stderr, "Line %d: \"%s\" -> ERROR (Could not tokenize statement, possibly memory error)\n", line_number, trimmed_line_ptr);
            continue;
        }

        struct StringGroupNode *tokens_for_parser = statement_tokens_head;
        int parse_success = tryParseSymbolRecursive(startSymbol, &tokens_for_parser, grammarRules);
        int all_tokens_consumed = (tokens_for_parser == NULL);

        printf("Line %d: \"%s\" -> %s\n", line_number, trimmed_line_ptr, (parse_success && all_tokens_consumed) ? "VALID" : "INVALID");

        freeStringGroupList(statement_tokens_head);
    }
    printf("--------------------------------------------------\n");
    printf("Finished validating statements.\n");

    fclose(statementsFile);
}

int main()
{
    char grammarFile[64] = "asd.txt";
    char statementFile[64] = "stmnt.txt";
    int valid; // This variable is part of the commented out section

    struct head *absHeadPtr = NULL;

    printf("--- Grammar File Setup ---\n");
    fileNameObtain(grammarFile);
    if (grammarFile[0] == '\0')
    {
        printf("No valid filename for grammar obtained. Exiting.\n");
        return -1;
    }
    valid = ftprVaild(grammarFile); // 'valid' would need to be declared if this is uncommented
    if (valid == -1)
    {
        return -1;
    }

    printf("\n--- Statement File Setup ---\n");
    fileNameObtain(statementFile);
    if (statementFile[0] == '\0')
    {
        printf("No valid filename for statement obtained. Exiting.\n");
        return -1;
    }
    valid = ftprVaild(statementFile); // 'valid' would need to be declared if this is uncommented
    if (valid == -1)
    {
        return -1;
    }

    printf("\nParsing grammar from: %s\n", grammarFile);
    absHeadPtr = parseGrammer(grammarFile);

    if (absHeadPtr == NULL)
    {
        printf("Error parsing grammar from file '%s' or the file is empty/invalid. Exiting.\n", grammarFile);
        return -1;
    }

    printGrammarList(absHeadPtr);
    printf("\n\n");

    if (statementFile[0] == '\0')
    {
        printf("Statement file name is empty. Skipping validation.\n");
    }
    else
    {
        validateStatementsFromFile(absHeadPtr, statementFile);
    }

    freeGrammarList(absHeadPtr);
    absHeadPtr = NULL;

    printf("\nProgram finished successfully.\n");
    return 0;
}
