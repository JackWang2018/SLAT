grammar slat;

WS  :   ( ' '
        | '\t'
        | '\r'
        | '\n'
	| COMMENT
        ) -> skip
    ;

LBRACE : '{';
RBRACE : '}';
LBRACKET : '[';
RBRACKET : ']';
LPAREN : '(';
RPAREN : ')';

COMMENT :   '#' ~[\r\n]*;
script : (WS | command)*;

command : (title_command | detfn_command | probfn_command | im_command | edp_command | fragfn_command | lossfn_command | compgroup_command | print_command | integration_command | recorder_command | analyze_command | set_command) ';' ;

title_command : 'title' STRING;

STRING : SINGLE_QUOTED_STRING | DOUBLE_QUOTED_STRING ;

fragment
SINGLE_QUOTED_STRING : SINGLE_QUOTE (SQ_UNESCAPED_CHAR| SQ_ESCAPED_CHAR)* SINGLE_QUOTE;

fragment
DOUBLE_QUOTED_STRING : DOUBLE_QUOTE (DQ_UNESCAPED_CHAR| DQ_ESCAPED_CHAR)* DOUBLE_QUOTE;

fragment
SINGLE_QUOTE : '\'';

fragment
SQ_UNESCAPED_CHAR : ~[\\\'];

fragment
SQ_ESCAPED_CHAR : '\\\'' | '\\\\';

fragment
DOUBLE_QUOTE : '"';

fragment
DQ_UNESCAPED_CHAR : ~[\\"];

fragment
DQ_ESCAPED_CHAR : '\\"' | '\\\\';


fragment
Nondigit : [a-zA-Z_];

fragment
Digit : [0-9];

INTEGER : [1-9][0-9]*;
    
ID :   Nondigit (Nondigit | Digit)*;
FLOAT_VAL: SIGN? FRACTION EXPONENT?;

fragment
SIGN: '-' | '+';

fragment
FRACTION : (DIGIT_SEQUENCE ('.' DIGIT_SEQUENCE?)?) | ('.' DIGIT_SEQUENCE);

fragment
DIGIT_SEQUENCE : Digit+;

fragment
EXPONENT : ('e' | 'E') SIGN? DIGIT_SEQUENCE;

detfn_command : 'detfn' ID 
	      (('hyperbolic' scalar3) |
	       ('powerlaw' scalar2));

scalar :  STRING | INTEGER| FLOAT_VAL | python_script | var_ref;
scalar2 : scalar ',' scalar;
scalar3 : scalar2 ',' scalar;
var_ref : '$' ID;

numerical_scalar : INTEGER | FLOAT_VAL | python_script | var_ref;

parameters : parameter | parameter_array | parameter_dictionary;

parameter : ID | STRING | INTEGER | FLOAT_VAL;

parameter_array : LBRACKET parameter (',' parameter)* RBRACKET;

parameter_dictionary : LPAREN dictionary_entry (',' dictionary_entry)* RPAREN;

dictionary_entry : (ID | STRING) ':' parameters;

probfn_command : 'probfn' ID 'lognormal' (ID | var_ref) ',' (ID | var_ref) lognormal_options;


im_command : 'im' ID ID;
edp_command : 'edp' ID ID ID;


fragfn_command : 'fragfn' ID 
	       (fragfn_db_params | fragfn_user_defined_params);
fragfn_db_params : (('--db' FILE_NAME)? ('--stdfunc' db_key)?) |
		   ('--stdfunc' db_key '--db' FILE_NAME) ;
		 
fragfn_user_defined_params : 
		  '[' scalar2 ']' (',' '[' scalar2 ']')*
                  lognormal_options;
mu_option : '--mu' ('mean_ln_x' | 'median_x' | 'mean_x');
sd_option : '--sd' ('sd_ln_x' | 'sd_x');
lognormal_options : (mu_option? sd_option?) | (sd_option mu_option);
	

placement_type : 'mean_ln_x' | 'mean_x' | 'median_x';
spread_type : 'sd_x' | 'sd_ln_x';

lognormal_dist : scalar2 lognormal_options;

lognormal_parameter_array : LBRACKET lognormal_dist (',' lognormal_dist)* RBRACKET;
array_array : LBRACKET parameter_array (',' parameter_array)* RBRACKET;	       
FILE_NAME : [a-zA-Z_.0-9]+;
db_key :  ID;

lossfn_command : 'lossfn' ID simple_loss_command;
simple_loss_command : ('--type' 'simple')? LBRACKET scalar2 RBRACKET (',' LBRACKET scalar2 RBRACKET)* lognormal_options;

lossfn_heading : 'cost' | 'disp' | 'upper_cost' | 'lower_cost' | 'lower_n' | 'upper_n' | 'mean_uncert' | 'var_uncert';
lossfn_headings : LPAREN lossfn_heading (',' lossfn_heading)* RPAREN;
lossfn_anon_array : LBRACKET parameter_array (',' parameter_array)* RBRACKET;
lossfn_dict : LPAREN lossfn_heading ':' parameter (',' lossfn_heading ':' parameter)* RPAREN;
lossfn_named_array : LBRACKET lossfn_dict (',' lossfn_dict)* RBRACKET;

compgroup_command : 'compgroup' ID ID ID ID INTEGER;

print_command : 'print' (('message' STRING?) |
	      ('detfn' ID) |
	      ('probfn' ID) |
	      ('im' ID) |
	      ('edp' ID) |
	      ('fragfn' ID) |
	      ('lossfn' ID) |
	      ('compgroup' ID)
	      )  print_options?;
print_options : FILE_NAME ('--append' | '--new')?;

integration_command : 'integration' integration_method numerical_scalar (INTEGER | var_ref | python_script);
integration_method : 'maq';

recorder_command : 'recorder' ((recorder_type ID recorder_at recorder_cols?) | ('dsrate' ID)) print_options?;
recorder_type : 'detfn' | 'probfn' | 'imrate' | 'edpim' | 'edprate' | 'dsedp'
	      | 'dsim' | 'lossds' | 'lossedp' | 'lossim';
recorder_at : (float_array | (FLOAT_VAL ':' FLOAT_VAL ':' FLOAT_VAL) | python_script | var_ref);
float_array : FLOAT_VAL (',' FLOAT_VAL)*;

col_spec : placement_type | spread_type | scalar;
recorder_cols : '--cols' col_spec (',' col_spec)*;

python_script : '$' LPAREN (non_paren_expression | balanced_paren_expression)* RPAREN;
non_paren_expression: ('=' | '/' | '+' | '*' | '-' | ~LPAREN)+;
balanced_paren_expression : LPAREN ('=' | ~RPAREN)* RPAREN;

analyze_command : 'analyze';

set_command : 'set' ID (python_script | var_ref | parameters);