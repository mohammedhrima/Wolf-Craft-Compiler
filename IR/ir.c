#include "parse.c"

// INSTRUCTIONS
Inst **first_insts;
Inst **insts;
size_t inst_size;
size_t inst_pos;
size_t reg_pos;

size_t bloc_index;
size_t str_index;
// STACK POINTER
size_t ptr;
size_t arg_ptr;

// SCOOP
Scoop *global_scoop;
size_t bloc_size;
ssize_t scoop_pos = -1;

Scoop *curr_scoop;
void enter_scoop(char *name)
{
    GLOG("ENTER SCOOP", "%s\n", name);
    if(global_scoop == NULL)
    {
        bloc_size = 10;
        global_scoop = calloc(bloc_size, sizeof(Scoop));
    }
    else if(scoop_pos + 1 == bloc_size)
    {
        Scoop *tmp = calloc(bloc_size * 2, sizeof(Scoop));
        memcpy(tmp, global_scoop, scoop_pos * sizeof(Scoop));
        bloc_size *= 2;
        free(global_scoop);
        global_scoop = tmp;
    }
    scoop_pos++;
    global_scoop[scoop_pos] = (Scoop){};
    global_scoop[scoop_pos].name = name;
    curr_scoop = &global_scoop[scoop_pos];
}

void exit_scoop()
{
    GLOG(" EXIT SCOOP", "%s\n", curr_scoop->name);
    free(curr_scoop->functions);
    free(curr_scoop->variables);
    global_scoop[scoop_pos] = (Scoop){};
    scoop_pos--;
    // if(scoop_pos >= 0)
    curr_scoop = &global_scoop[scoop_pos];
    // else
        // curr_scoop = NULL;
}

#if IR
void add_inst(Inst *inst)
{
    if (first_insts == NULL)
    {
        inst_size = 100;
        first_insts = calloc(inst_size, sizeof(Inst *));
    }
    else if (inst_pos + 1 == inst_size)
    {
        Inst **tmp = calloc(inst_size * 2, sizeof(Inst *));
        memcpy(tmp, first_insts, inst_pos * sizeof(Inst *));
        free(first_insts);
        first_insts = tmp;
        inst_size *= 2;
    }
    first_insts[inst_pos++] = inst;
}

// NAMESPACE
Token *get_namespace(char *name) // TODO: maybe you nee ne to use get scoop
{
    return NULL;
}

// VARIABLES / FUNCTIONS
Token *get_variable(char *name)
{
    CLOG("get var", "%s\n", name);
    CLOG("Scoop", "%s\n", curr_scoop->name);
    for(size_t i = 0; i < curr_scoop->var_pos; i++)
    {
        Token *var = curr_scoop->variables[i];
        if(strcmp(var->name, name) == 0)
        {
            GLOG("found", "in [%s] scoop\n", curr_scoop->name);
            return curr_scoop->variables[i];
        }
    }
    RLOG(FUNC, "'%s' Not found\n", name);
    exit(1);
    return NULL;
}

Token *new_variable(Token *token)
{
    CLOG("new var", "%s\n", token->name);
    // TODO: check here the global variables
    Token **variables = curr_scoop->variables;
    CLOG("Scoop", "%s\n", curr_scoop->name);
    for(size_t i = 0; i < curr_scoop->var_pos; i++)
    {
        Token *var = curr_scoop->variables[i];
        if(strcmp(var->name, token->name) == 0)
        {
            debug("%sRedefinition of %s%s\n", RED, token->name, RESET);
            exit(1);
        }
    }
    if(curr_scoop->variables == NULL)
    {
        curr_scoop->var_size = 10;
        curr_scoop->variables = calloc(curr_scoop->var_size, sizeof(Token*));
    }
    else if(curr_scoop->var_pos + 1 == curr_scoop->var_size)
    {
        curr_scoop->var_size *= 2;
        Token **tmp = calloc(curr_scoop->var_size, sizeof(Token*));
        memcpy(tmp, curr_scoop->variables, curr_scoop->var_pos * sizeof(Token*));
        free(curr_scoop->variables);
        curr_scoop->variables = tmp;
    }
    // GLOG("", "in [%s] scoop\n", curr_scoop->name);
    curr_scoop->variables[curr_scoop->var_pos++] = token;
    return token;
}

Node *get_function(char *name)
{
    // TODO: remove output from here
    CLOG("get func", "%s\n", name);
    CLOG("Scoop", "%s\n", curr_scoop->name);
    char *builtins[] = {"output", 0};
    for(int i = 0; builtins[i]; i++)
        if(strcmp(name, builtins[i]) == 0)
            return NULL;
    for(ssize_t j = scoop_pos; j >= 0; j--)
    {
        Scoop *scoop = &global_scoop[j];
        for(size_t i = 0; i < scoop->func_pos; i++)
        {
            Node *func = scoop->functions[i];
            if(strcmp(func->token->name, name) == 0)
                return func;
        }
    }
    RLOG(FUNC, "'%s' Not found\n", name);
    exit(1);
    return NULL;
}

// TODO: create a list fo built in functions
Node *new_function(Node *node)
{
    CLOG("new func", "%s\n", node->token->name);
    CLOG("Scoop", "%s\n", curr_scoop->name);
    char *builtins[] = {"output", 0};
    for(int i = 0; builtins[i]; i++)
    {
        if(strcmp(node->token->name, builtins[i]) == 0)
        {
            error("%s is a built in function\n", node->token->name);
            exit(1);
        }
    }
    for(size_t i = 0; i < curr_scoop->func_pos; i++)
    {
        Node *func = curr_scoop->functions[i];
        if(strcmp(func->token->name, node->token->name) == 0)
        {
            error("Redefinition of %s\n", node->token->name);
            exit(1);
        }
    }
    if(curr_scoop->functions == NULL)
    {
        curr_scoop->func_size = 10;
        curr_scoop->functions = calloc(curr_scoop->func_size, sizeof(Node*));
    }
    else if(curr_scoop->func_pos + 1 == curr_scoop->func_size)
    {
        curr_scoop->func_size *= 2;
        Node **tmp = calloc(curr_scoop->func_size, sizeof(Node*));
        memcpy(tmp, curr_scoop->functions, curr_scoop->func_pos * sizeof(Node*));
        free(curr_scoop->functions);
        curr_scoop->functions = tmp;
    }
    curr_scoop->functions[curr_scoop->func_pos++] = node;
    return node;
}


Inst *new_inst(Token *token)
{
    debug("new instruction has type %s\n", to_string(token->type));
    Inst *new = calloc(1, sizeof(Inst));
    new->token = token;
    if (token->name && token->declare)
    {
        new_variable(token);
        // if (token->declare)
        {
            // if(token->isarg)
            //     token->ptr = (arg_ptr += 8);
            // else
            token->ptr = (ptr += 8);
            token->reg = ++reg_pos;
        }
    }
    else
    {
        switch (token->type)
        {
        case add_: case sub_: case mul_: case div_: case equal_:
        case less_: case more_: case less_equal_: case more_equal_:
        case not_equal_: case fcall_: 
            token->reg = ++reg_pos;
            break;
        default:
            break;
        }
    }
    add_inst(new);
    return new;
}

Token *generate_ir(Node *node)
{
    // debug("gen-ir: %s\n", to_string(node->token->type));
    Inst *inst = NULL;
    switch (node->token->type)
    {
    case id_:
    {
        Token *token = get_variable(node->token->name);
        return token;
        break;
    }
    case module_:
    {
        return node->token;
        break;
    }
    case if_:
    {
        Node *tmp = node;
        Node *curr = node->left;

        // condition
        Token *result = generate_ir(curr->left); // TODO: check if it's boolean

        node->token->type = jne_;
        node->token->name = strdup("endif");
        node->token->index = ++bloc_index;

        Token *lastInst = copy_token(node->token);
        new_inst(lastInst); // jne to endif

        curr = curr->right;
        while(curr->left) // if code bloc
        {
            generate_ir(curr->left);
            curr = curr->right;
        }
        Inst *endInst = NULL;
        if(node->right)
        {
            endInst = new_inst(new_token("endif", 0, 5, node->token->space, jmp_));
            endInst->token->index = node->token->index;
        }

        curr = node->right;
        while(curr)
        {
            if(curr->left->token->type == elif_)
            {
                curr->left->token->index = ++bloc_index;
                curr->left->token->type = bloc_;
                curr->left->token->name = strdup("elif");

                {
                    free(lastInst->name);
                    lastInst->name = strdup("elif");
                    lastInst->index = curr->left->token->index;
                    lastInst = copy_token(lastInst);
                }

                new_inst(curr->left->token);
                Node *tmp = curr->left;
                generate_ir(tmp->left); // elif condition, TODO: check is boolean
                
                new_inst(lastInst);

                tmp = tmp->right;
                while(tmp->left)
                {
                    generate_ir(tmp->left);
                    tmp = tmp->right;
                }
            }
            else if(curr->left->token->type == else_)
            {
                curr->left->token->index = ++bloc_index;
                curr->left->token->type = bloc_;
                curr->left->token->name = strdup("else");
                new_inst(curr->left->token);

                {  
                    free(lastInst->name);
                    lastInst->name = strdup("else");
                    lastInst->index = curr->left->token->index;
                    lastInst = copy_token(lastInst);
                }

                Node *tmp = curr->left;
                tmp = tmp->right;
                while(tmp->left)
                {
                    generate_ir(tmp->left);
                    tmp = tmp->right;
                }
                break;
            }
            if(curr->right)
            {
                endInst = new_inst(new_token("endif", 0, 5, node->token->space, jmp_));
                endInst->token->index = node->token->index;
            }
            curr = curr->right;
        }

        Token *new = new_token("endif", 0, 5, node->token->space, bloc_);

        new->index = node->token->index;
        new_inst(new);
        // free_token(lastInst);
        return node->left->token;
        break;
    }
    case while_:
    {
        // condition
        node->token->type = bloc_;
        node->token->name = strdup("while");
        node->token->index = ++bloc_index;
        inst = new_inst(node->token);

        Token *result = generate_ir(node->left); // TODO: check if it's boolean
        Token *end = copy_token(node->token);
        end->type = jne_;
        if(end->name) free(end->name);
        end->name = strdup("endwhile");
        new_inst(end);

        Node *curr = node->right;
        while(curr) // if code bloc
        {
            generate_ir(curr->left);
            curr = curr->right;
        }

        Token *lastInst = copy_token(node->token);
        lastInst->type = jmp_;
        free(lastInst->name);
        lastInst->name = strdup("while");
        new_inst(lastInst); // jne to endif

        lastInst = copy_token(node->token);
        lastInst->type = bloc_;
        free(lastInst->name);
        lastInst->name = strdup("endwhile");
        new_inst(lastInst); // jne to endif
        break;
    }
    case fdec_:
    {
        new_function(node);
        enter_scoop(node->token->name);
        Token *fcall = copy_token(node->token);

        fcall->type = fdec_;
        inst = new_inst(fcall);
        Node *curr;
        
        pnode(node, NULL, 0);
        // inside_function = true;
        if(node->left) // arguments
        {
            char *regs[] = {"rdi", "rsi", "rdx", "rcx", NULL};
            int i = 0;
            size_t ptr = 8;
            
            Token **list = NULL;
            size_t list_pos = 0;
            size_t list_size = 0;

            curr = node->left->right;
            while(curr && curr->left)
            {
                Inst *inst = new_inst(new_token(NULL, 0, 0, node->token->space, pop_));
                inst->left = generate_ir(curr->left);
                curr->left->token->declare = false;
                // inst->left = curr->left->token;
                if(regs[i])
                {
                    inst->right = new_token(regs[i], 0, strlen(regs[i]), node->token->space, 0);
                    i++;
                }
                else
                {
                    inst->right = new_token(NULL, 0, 0, node->token->space, 0);
                    inst->right->ptr = (ptr += 8);
                    if(list == NULL)
                    {
                        list_size = 10;
                        list = calloc(list_size, sizeof(Token*));
                    }
                    else if(list_pos + 1 == list_size)
                    {
                        Token **tmp = calloc(list_size * 2, sizeof(Token*));
                        memcpy(tmp, list, list_pos * sizeof(Token*));
                        free(list);
                        list_size *= 2;
                        list = tmp;
                    }
                    list[list_pos++] = inst->right;
                }
                // Token *arg = generate_ir(curr->left);
                curr = curr->right;
            }
            i = 0;
            while(i < list_pos / 2)
            {
                size_t tmp = list[i]->ptr;
                list[i]->ptr = list[list_pos - i - 1]->ptr;
                list[list_pos - i - 1]->ptr = tmp;
                i++;
            }
            free(list);
        }
        curr = node->right;
        while(curr)
        {
            generate_ir(curr->left);
            curr = curr->right;
        }
        exit_scoop();
        break;
    }
    case ret_:
    {
        inst = new_inst(node->token);
        inst->left = generate_ir(node->left);
        break;
    }
    case fcall_:
    {
        // pnode(node, NULL, 0);
        size_t tmp_arg_ptr = arg_ptr;
        size_t tmp_ptr = ptr;
        arg_ptr = 8;
        ptr = 8; // TODO: to be checked
        if(strcmp(node->token->name, "output") == 0)
        {
            Node *curr = node;
            char *fname = NULL;
#if 0
            while(curr->left)
            {
                pnode(curr->left, NULL, 0);
                curr = curr->right;
            }
#endif
            // exit(1);
            curr = node;
            while(curr->left)
            {
                // debug("loop\n");
                // Node *arg = curr->left;
                Token *left = generate_ir(curr->left);
                fname = NULL;
                switch(left->type)
                {
                    // TODO: add other types / maybe you will remove it
                    case chars_: fname = ".putstr"; break;
                    case int_:    fname = ".putnbr"; break;
                    default: 
                        RLOG(FUNC, "%d: handle this case <%s>\n", LINE, to_string(left->type)); 
                        exit(1);
                }
                if(fname)
                {
                    inst = new_inst(new_token(NULL, 0, 0, node->token->space, push_));
                    // TODO: it causes problem in output("hello world")
                    // left = copy_token(left);
                    left->declare = false;
                    inst->left = left;
                    inst->right = new_token("rdi", 0, 3, node->token->space, 0);
                    // new_inst(left);
                    inst = new_inst(new_token(fname, 0, strlen(fname), node->token->space, fcall_));
                    inst->token->isbuiltin = true;
                    inst->left = left;
                }
                curr = curr->right;
            }
        }
        else 
        {
            Node *func = get_function(node->token->name);
            Node *arg = func->left->right;
            debug("has the following arguments\n");
            while(arg)
            {
                ptoken(arg->left->token);
                arg = arg->right;
            }
            char *regs[] = {"rdi", "rsi", "rdx", "rcx", NULL};
            int i = 0;
            size_t ptr = 8;
        
            Node *curr = node;
            arg = func->left->right;
            while(curr->left)
            {
                Token *left = generate_ir(curr->left);
                Inst *inst = new_inst(new_token(NULL, 0, 0, node->token->space, push_));
                inst->left = left;
                // debug("%s => %s\n", 
                // to_string(inst->left->type), 
                // to_string(arg->left->token->type));
                if
                (
                    inst->left->type != arg->left->token->type && 
                    inst->left->retType != arg->left->token->type
                )
                {
                    error("Incompatible type for function call <%s>\n", func->token->name);
                    // TODO: add line after
                    exit(1);
                }
                if(regs[i])
                {
                    inst->right = new_token(regs[i], 0, strlen(regs[i]), node->token->space, 0);
                    i++;
                }
                else
                    inst->right = new_token(NULL, 0, 0, node->token->space, 0);
                // Token *arg = generate_ir(curr->left);
                curr = curr->right;
                arg = arg->right;
            }
            new_inst(node->token);
            // exit(1);
        }
        arg_ptr = tmp_arg_ptr;
        ptr = tmp_ptr;
        return node->token;
        break;
    }
    case dot_:
    {
        // TODO: use namespaces here
        debug("found dot\n");
        Token *left = generate_ir(node->left);
        if(left->type == module_)
        {
            // is module
            // TODO: module must have a valid name like "sys"
            debug("left is module\n");
            Token *right = generate_ir(node->right);


            // exit(1);
        }
        else
        {
            // is attribute or method
            error("handle this case in dot\n");
        }
        return left;
        // exit(1);
        break;
    }
    case bool_: case int_: case chars_: case char_: case float_: case void_:
    {
        inst = new_inst(node->token);
        break;
    }
    case assign_:
    case add_: case sub_: case div_: case mul_:
    case not_equal_: case equal_: case less_: 
    case more_: case less_equal_: case more_equal_:
    {
        Token *left = generate_ir(node->left);
        Token *right = generate_ir(node->right);
        // TODO: fix this later for cases like char + int
        if(left->type != right->type && left->type != right->retType)
        {
            error("Incompatible type for <%s> and <%s>",
            to_string(left->type), to_string(right->type));
            exit(1);
        }
        inst = new_inst(node->token);
        inst->left = left;
        inst->right = right;
        switch(node->token->type)
        {
            case assign_:
                node->token->retType = left->type; break;
            case add_: case sub_: case mul_: case div_: // TODO: check mul between string and int
                node->token->retType = left->type; node->token->c = 'a'; break;
            case not_equal_: case equal_: case less_: 
            case more_: case less_equal_: case more_equal_:
                node->token->retType = bool_; break;
            default: break;
        }
        // inst->token->type = left->type; // TODO: to be checked
        break;
    }
    default: {RLOG(FUNC, "%d: handle this case\n", LINE); exit(1); break;} 
    }
    return inst->token;
}

void print_ir()
{
    debug(SPLIT);
    int j = 0;
    if(insts == NULL)
    {
        printf("insts is NULL\n");
        exit(1);
    }
    for (int i = 0; insts[i]; i++)
    {
        Token *curr = insts[i]->token;
        Token *left = insts[i]->left;
        Token *right = insts[i]->right;
        if (curr->remove)
        {
            RLOG(FUNC, "this condtion must nuver exists\n");
            exit(1);
            continue;
        }
        switch (curr->type)
        {
        case assign_:
        {
            curr->reg = left->reg;
            debug("r%.2d: %s [%s] ", curr->reg, to_string(curr->type), to_string(left->type));
            debug("%s in (%d) to ", left->name, left->reg);
            if (right->reg)
                debug("r%.2d", right->reg);
            else
            {
                switch (right->type)
                { // TODO: handle the other cases
                case int_: debug("%lld", right->Int.value); break;
                case bool_: debug("%s", right->Bool.value ? "True" : "False"); break;
                case float_: debug("%f", right->Float.value); break;
                case char_: debug("%c", right->Char.value); break;
                case chars_: debug("%s", right->Chars.value); break;
                default: RLOG(FUNC, "%d: handle this case\n", LINE); exit(1); break;
                }
            }
            debug("\n");
            break;
        }
        case fcall_:
        {
            debug("r%.2d: call %s\n",curr->reg, curr->name);
            break;
        }
        case add_: case sub_: case mul_: case div_: case equal_:
        case less_: case more_: case less_equal_: case more_equal_:
        case not_equal_:
        {
            // TODO: set invalid operation for boolean type
            debug("r%.2d: %s ", curr->reg, to_string(curr->type));
            if (left->reg)
                debug("r%.2d", left->reg);
            else
                switch(left->type)
                {
                    case int_: debug("%lld", left->Int.value); break;
                    case char_: debug("%c", left->Char.value); break;
                    case chars_: debug("%s", left->Chars.value); break;
                    case float_: debug("%f", left->Float.value); break;
                    default: break;
                }

            if (left->name)
                debug(" (%s)", left->name);
            debug(" to ");
            if (right->reg)
                debug("r%.2d", right->reg);
            else
                switch(right->type)
                {
                    case int_: debug("%lld", right->Int.value); break;
                    case char_: debug("%c", right->Char.value); break;
                    case chars_: debug("%s", right->Chars.value); break;
                    case float_: debug("%f", right->Float.value); break;;
                    default: break;
                }

            if (right->name)
                debug(" (%s)", right->name);
            debug("\n");
            break;
        }
        case int_: case char_: case bool_: case chars_: case float_:
        {
            debug("r%.2d: [%s] ", curr->reg, to_string(curr->type));
            if (curr->declare) 
                debug("declare %s ", curr->name);
            else if(curr->name)
                debug("variable %s ", curr->name);
            else if(curr->type == int_)
                debug("value %lld ", curr->Int.value);
            else if(curr->type == char_)
                debug("value %c ", curr->Char.value);
            else if(curr->type == bool_)
                debug("value %s ", curr->Bool.value ? "True" : "False");
            else if(curr->type == float_)
            {
                curr->index = ++str_index;
                debug("value %f ", curr->Float.value);
            }
            else if(curr->type == chars_)
            {
                curr->index = ++str_index;
                debug("value %s in STR%zu ", curr->Chars.value, curr->index);
            }
            else
                error("handle this case in generate ir line %d\n", LINE);
            // if(curr->isarg)
            //     debug(" [argument]");
            debug("\n");
            break;
        }
        case push_:
        {
            // TODO: check all cases
            debug("rxx: push ");
            // if(left->ptr)
            debug("PTR r%.2d ", left->reg);
            if(right->name)
                debug("to %s", right->name);
            debug("\n");
            break;
        }
        case pop_:
        {
            // TODO: check all cases
            debug("rxx: pop ");
            if(left->ptr)
                debug("PTR [%zu] ", left->ptr);
            debug("from ");
            if(right->name)
                debug("%s", right->name);
            else
                debug("[%zu]", right->ptr);
            debug("\n");
            break;
        }
        case ret_:
        {
            /*
                TODO:
                    + if function has datatype must have return
                    + return value must be compatible with function
            */
            debug("rxx: return "); 
            ptoken(left);
            break;
        }
        case jne_:  debug("rxx: jne %s%zu\n", curr->name, curr->index); break;
        case jmp_:  debug("rxx: jmp %s%zu\n", curr->name, curr->index); break;
        case bloc_: debug("rxx: %s%zu (bloc)\n", curr->name, curr->index); break;
        case fdec_: debug("%s: (func dec)\n", curr->name); break;
        default: 
            debug("%sPrint IR: Unkown inst [%s]%s\n", RED, to_string(curr->type), RESET);
            break;
        }
        j++;
    }
    debug("%.2d Instructions on total\n", j);
    debug(SPLIT);
}

bool optimize_ir(int op_index)
{
#if 1
    bool did_optimize = false;
    switch (op_index)
    {
    case 0:
    {
        debug("OPTIMIZATION %d (calculate operations on constant type 0)\n", op_index);
        for (int i = 0; insts[i]; i++)
        {
            Token *token = insts[i]->token;
            Token *left = insts[i]->left;
            Token *right = insts[i]->right;
            if (check_type((Type[]){add_, sub_, mul_, div_, 0}, insts[i]->token->type))
            {
                if
                (
                    // TODO: check it left nad right are compatible
                    check_type((Type[]){int_, float_, chars_, 0}, left->type) &&
                    check_type((Type[]){int_, float_, chars_, 0}, right->type) &&
                    !left->name && !right->name
                )
                {
                    
                    switch(left->type)
                    {
                    case int_:
                        switch (token->type)
                        {
                        case add_: token->Int.value = left->Int.value + right->Int.value; break;
                        case sub_: token->Int.value = left->Int.value - right->Int.value; break;
                        case mul_: token->Int.value = left->Int.value * right->Int.value; break;
                        case div_: token->Int.value = left->Int.value / right->Int.value; break;
                        default: break;
                        }
                        break;
                    case float_:
                        switch (token->type)
                        {
                        case add_: token->Float.value = left->Float.value + right->Float.value; break;
                        case sub_: token->Float.value = left->Float.value - right->Float.value; break;
                        case mul_: token->Float.value = left->Float.value * right->Float.value; break;
                        case div_: token->Float.value = left->Float.value / right->Float.value; break;
                        default: break;
                        }
                    // case chars_:
                    //     switch(token->type)
                    //     {
                    //     case add_: token->Chars.value = strjoin(left, right); break;
                    //     default:
                    //         error("Invalid %s op in chars\n", to_string(token->type)); break;
                    //     }
                    default:
                        error("%s: %d handle this case\n", FUNC, LINE);
                        break;
                    }
                    token->type = left->type;
                    left->remove = true;
                    right->remove = true;
                    insts = copy_insts(first_insts, insts, inst_pos, inst_size);
                    i = 0;
                    did_optimize = true;
                }
            }
            if (check_type((Type[]){int_, float_, chars_, bool_, char_, 0}, token->type) && !token->name)
            {
                token->c = 0;
                // debug("found to remove in r%d\n", token->r1);
                token->remove = true;
                insts = copy_insts(first_insts, insts, inst_pos, inst_size);
                i = 0;
                did_optimize = true;
            }
        }
        break;
    }
    case 1:
    {
        // TODO: do comparision operation on numbers etc...
        debug("OPTIMIZATION %d (calculate operations on numbers type 1)\n", op_index);
        int i = 1;
        while (insts[i])
        {
            Token *token = insts[i]->token;
            Token *left = insts[i]->left;
            Token *right = insts[i]->right;
            if (
                //  TODO: handle string also here X'D ma fiyach daba
                check_type((Type[]){add_, sub_, mul_, div_, 0}, token->type) &&
                insts[i - 1]->token->type == add_ &&
                left == insts[i - 1]->token &&
                !insts[i - 1]->right->name &&
                !right->name)
            {
                // debug("%sfound %s\n", RED, RESET);
                token->remove = true;
                switch(token->type)
                {
                case add_: insts[i - 1]->right->Int.value += right->Int.value; break;
                case sub_: insts[i - 1]->right->Int.value -= right->Int.value; break;
                case mul_: insts[i - 1]->right->Int.value *= right->Int.value; break;
                case div_: insts[i - 1]->right->Int.value /= right->Int.value; break;
                default: break;
                }
                // debug("value is %lld\n", insts[i - 1]->right->Int.value);
                if (insts[i + 1]->left == token) insts[i + 1]->left = insts[i - 1]->token;
                i = 1;
                insts = copy_insts(first_insts, insts, inst_pos, inst_size);
                did_optimize = true;
                continue;
            }
            i++;
        }
        break;
    }
    case 2:
    {
        debug("OPTIMIZATION %d (remove reassigned variables)\n", op_index);
        for (int i = 0; insts[i]; i++)
        {
            if (insts[i]->token->declare)
            {
                int j = i + 1;
                while (insts[j] && insts[j]->token->space == insts[i]->token->space)
                {
                    ptoken(insts[j]->token);
                    if (insts[j]->token->type == assign_ && insts[j]->left == insts[i]->token)
                    {
                        insts[i]->token->declare = false;
                        insts[i]->token->remove = true;
                        did_optimize = true;
                        break;
                    }
                    if 
                    (
                        insts[j]->left && insts[j]->left &&
                        (
                        insts[j]->left->reg == insts[i]->token->reg || 
                        insts[j]->right->reg == insts[i]->token->reg
                        )
                    )
                        break;
                    j++;
                }
            }
            else if (insts[i]->token->type == assign_)
            {
                int j = i + 1;
                while (insts[j] && insts[j]->token->space == insts[i]->token->space)
                {
                    if(!insts[j]->left || !insts[j]->right || !insts[i]->token)
                    {
                        j++;
                        continue;
                    }
                    if 
                    (
                        insts[j]->token->type == assign_ && 
                        insts[j]->left == insts[i]->left
                    )
                    {
                        insts[i]->token->remove = true;
                        did_optimize = true;
                        break;
                    }
                    // if the variable is used some where
                    else if 
                    (
                        insts[j]->left->reg == insts[i]->token->reg || 
                        insts[j]->right->reg == insts[i]->token->reg
                    )
                        break;
                    j++;
                }
            }
        }
        break;
    }
    case 3:
    {
        debug("OPTIMIZATION %d (remove unused instructions)\n", op_index);
        for(int i = 1; insts[i]; i++)
        {
            if
            (
                check_type((Type[]){add_, sub_, mul_, div_, 0}, insts[i - 1]->token->type) &&
                insts[i]->left->reg != insts[i - 1]->token->reg && 
                insts[i]->right->reg != insts[i - 1]->token->reg
            )
            {
                did_optimize = true;
                insts[i - 1]->token->remove = true;
                insts = copy_insts(first_insts, insts, inst_pos, inst_size);
                i = 1;
            }
        }
        break;
    }
    default:
        break;
    }
    return did_optimize;
#endif
}

#endif

