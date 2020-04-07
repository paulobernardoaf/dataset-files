#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "nvim/types.h"
#include "nvim/viml/parser/parser.h"
#include "nvim/eval/typval.h"
typedef enum {
kCCStrategyUseOption = 0, 
kCCStrategyMatchCase = '#',
kCCStrategyIgnoreCase = '?',
} ExprCaseCompareStrategy;
typedef enum {
kExprLexInvalid = 0, 
kExprLexMissing, 
kExprLexSpacing, 
kExprLexEOC, 
kExprLexQuestion, 
kExprLexColon, 
kExprLexOr, 
kExprLexAnd, 
kExprLexComparison, 
kExprLexPlus, 
kExprLexMinus, 
kExprLexDot, 
kExprLexMultiplication, 
kExprLexNot, 
kExprLexNumber, 
kExprLexSingleQuotedString, 
kExprLexDoubleQuotedString, 
kExprLexOption, 
kExprLexRegister, 
kExprLexEnv, 
kExprLexPlainIdentifier, 
kExprLexBracket, 
kExprLexFigureBrace, 
kExprLexParenthesis, 
kExprLexComma, 
kExprLexArrow, 
kExprLexAssignment, 
} LexExprTokenType;
typedef enum {
kExprCmpEqual, 
kExprCmpMatches, 
kExprCmpGreater, 
kExprCmpGreaterOrEqual, 
kExprCmpIdentical, 
} ExprComparisonType;
typedef enum {
kExprOptScopeUnspecified = 0,
kExprOptScopeGlobal = 'g',
kExprOptScopeLocal = 'l',
} ExprOptScope;
typedef enum {
kExprAsgnPlain = 0, 
kExprAsgnAdd, 
kExprAsgnSubtract, 
kExprAsgnConcat, 
} ExprAssignmentType;
#define EXPR_OPT_SCOPE_LIST ((char[]){ kExprOptScopeGlobal, kExprOptScopeLocal })
typedef enum {
kExprVarScopeMissing = 0,
kExprVarScopeScript = 's',
kExprVarScopeGlobal = 'g',
kExprVarScopeVim = 'v',
kExprVarScopeBuffer = 'b',
kExprVarScopeWindow = 'w',
kExprVarScopeTabpage = 't',
kExprVarScopeLocal = 'l',
kExprVarScopeArguments = 'a',
} ExprVarScope;
#define EXPR_VAR_SCOPE_LIST ((char[]) { kExprVarScopeScript, kExprVarScopeGlobal, kExprVarScopeVim, kExprVarScopeBuffer, kExprVarScopeWindow, kExprVarScopeTabpage, kExprVarScopeLocal, kExprVarScopeBuffer, kExprVarScopeArguments, })
typedef struct {
ParserPosition start;
size_t len;
LexExprTokenType type;
union {
struct {
ExprComparisonType type; 
ExprCaseCompareStrategy ccs; 
bool inv; 
} cmp; 
struct {
enum {
kExprLexMulMul, 
kExprLexMulDiv, 
kExprLexMulMod, 
} type; 
} mul; 
struct {
bool closing; 
} brc; 
struct {
int name; 
} reg; 
struct {
bool closed; 
} str; 
struct {
const char *name; 
size_t len; 
ExprOptScope scope; 
} opt; 
struct {
ExprVarScope scope; 
bool autoload; 
} var; 
struct {
LexExprTokenType type; 
const char *msg; 
} err; 
struct {
union {
float_T floating;
uvarnumber_T integer;
} val; 
uint8_t base; 
bool is_float; 
} num; 
struct {
ExprAssignmentType type;
} ass; 
} data; 
} LexExprToken;
typedef enum {
kELFlagPeek = (1 << 0),
kELFlagForbidScope = (1 << 1),
kELFlagAllowFloat = (1 << 2),
kELFlagIsNotCmp = (1 << 3),
kELFlagForbidEOC = (1 << 4),
} LexExprFlags;
typedef enum {
kExprNodeMissing = 0,
kExprNodeOpMissing,
kExprNodeTernary, 
kExprNodeTernaryValue, 
kExprNodeRegister, 
kExprNodeSubscript, 
kExprNodeListLiteral, 
kExprNodeUnaryPlus,
kExprNodeBinaryPlus,
kExprNodeNested, 
kExprNodeCall, 
kExprNodePlainIdentifier,
kExprNodePlainKey,
kExprNodeComplexIdentifier,
kExprNodeUnknownFigure,
kExprNodeLambda, 
kExprNodeDictLiteral, 
kExprNodeCurlyBracesIdentifier, 
kExprNodeComma, 
kExprNodeColon, 
kExprNodeArrow, 
kExprNodeComparison, 
kExprNodeConcat,
kExprNodeConcatOrSubscript,
kExprNodeInteger, 
kExprNodeFloat, 
kExprNodeSingleQuotedString,
kExprNodeDoubleQuotedString,
kExprNodeOr,
kExprNodeAnd,
kExprNodeUnaryMinus,
kExprNodeBinaryMinus,
kExprNodeNot,
kExprNodeMultiplication,
kExprNodeDivision,
kExprNodeMod,
kExprNodeOption,
kExprNodeEnvironment,
kExprNodeAssignment,
} ExprASTNodeType;
typedef struct expr_ast_node ExprASTNode;
struct expr_ast_node {
ExprASTNodeType type; 
ExprASTNode *children;
ExprASTNode *next;
ParserPosition start;
size_t len;
union {
struct {
int name; 
} reg; 
struct {
struct {
bool allow_dict;
bool allow_lambda;
bool allow_ident;
} type_guesses;
size_t opening_hl_idx;
} fig; 
struct {
ExprVarScope scope; 
const char *ident;
size_t ident_len; 
} var; 
struct {
bool got_colon; 
} ter; 
struct {
ExprComparisonType type; 
ExprCaseCompareStrategy ccs; 
bool inv; 
} cmp; 
struct {
uvarnumber_T value;
} num; 
struct {
float_T value;
} flt; 
struct {
char *value;
size_t size;
} str; 
struct {
const char *ident; 
size_t ident_len; 
ExprOptScope scope; 
} opt; 
struct {
const char *ident; 
size_t ident_len; 
} env; 
struct {
ExprAssignmentType type;
} ass; 
} data;
};
enum ExprParserFlags {
kExprFlagsMulti = (1 << 0),
kExprFlagsDisallowEOC = (1 << 1),
kExprFlagsParseLet = (1 << 2),
};
typedef struct {
const char *msg;
const char *arg;
int arg_len;
} ExprASTError;
typedef struct {
ExprASTError err;
ExprASTNode *root;
} ExprAST;
extern const uint8_t node_maxchildren[];
extern const char *const east_node_type_tab[];
extern const char *const eltkn_cmp_type_tab[];
extern const char *const ccs_tab[];
extern const char *const expr_asgn_type_tab[];
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "viml/parser/expressions.h.generated.h"
#endif
