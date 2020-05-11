#include "tcc.h"

// Edit here to add type
// - Must be in order with TypeKind declaration.
// - Must be same size as declaration.
char *typenames[] = {
  "char", "int",
};

int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}

Type *new_type(TypeKind kind, int size, int align){
  Type *type = calloc(1, sizeof(Type));
  type->kind = kind;
  type->size = size;
  type->align = align;
  return type;
}

Type *new_array_type(Type *ptr_to, int length){
  Type *arr = new_type(TY_ARRAY, ptr_to->size * length, ptr_to->align);
  arr->array_len = length;
  arr->ptr_to = ptr_to;
  return arr;
}

Type *new_struct_type(Member *members){
  // calc offset & size
  int offset = 0;
  int align = 0;
  for (Member *mem = members; mem; mem = mem->next){
    offset = align_to(offset, mem->type->align);
    mem->offset = offset;
    offset += mem->type->size;
    if (align < mem->type->align){
      align = mem->type->align;
    }
  }
  Type *st = new_type(TY_STRUCT, align_to(offset, align), align);
  st->members = members;

  return st;
}

Type *new_int_type(){
  return new_type(TY_INT, 4, 4);
}

Type *new_char_type(){
  return new_type(TY_INT, 1, 1);
}

Type *new_func_type(Type *return_type){
  Type *type = new_type(TY_FUNC, 1, 1);
  type->return_type = return_type;
  return type;
}

Type *pointer_to(Type *type){
  Type *ty = new_type(TY_PTR, 8, 8);
  ty->ptr_to = type;
  return ty;
}

Member *struct_member(){
  Type *type = expect_type();
  type = consume_pointer(type);
  char *name = expect_ident();
  type = read_type_suffix(type);

  Member *member = calloc(1, sizeof(Member));
  member->type = type;
  member->name = name;

  expect(";");
  return member;
}

TagScope *find_tag(Token *tok){
  for (TagScope *sc = tag_scope; sc; sc = sc->next){
    if (strlen(sc->name) == tok->len && !strncmp(tok->str, sc->name, tok->len)){
      return sc;
    }
  }
  return NULL;
}

void push_tag(Token *tok, Type *type){
  TagScope *sc = calloc(1, sizeof(TagScope));
  sc->name = substr(tok->str, tok->len);
  sc->type = type;
  sc->next = tag_scope;
  tag_scope = sc;
}

Type *expect_type(){
  for (int i = 0; i < sizeof(typenames) / sizeof(*typenames); i++){
    if (consume(typenames[i])){
      Type *ty;
      // Edit here to add type
      switch (i){
        case TY_CHAR:
          ty = new_char_type();
          break;
        case TY_INT:
          ty = new_int_type();
          break;
      }
      ty = consume_pointer(ty);
      return ty;
    }
  }
  if (consume("struct")){
    Token *tag = consume_ident();
    if (tag && (! peek("{"))){
      // struct foo bar
      TagScope *sc = find_tag(tag);
      if (! sc){
        error_token(token, "Undefined struct tag");
      }
      Type *ty = sc->type;
      ty = consume_pointer(ty);
      return ty;
    }
    // struct {} or struct foo {}
    expect("{");
    Member head = {};
    Member *cur = &head;
    while (! consume("}")){
      cur->next = struct_member();
      cur = cur->next;
    }
    Member *members = head.next;
    Type *ty = new_struct_type(members);

    if (tag){
      push_tag(tag, ty);
    }
    
    return ty;
  }

  Token *ident = consume_ident();
  if (ident){
    VarScope *vsc = find_var(ident);
    if (vsc->type_def != NULL){
      Type *type = consume_pointer(vsc->type_def);
      return type;
    }
  }

  error_at(token->str, "expected any type");
}

bool peek_type(){
  Token *saved = token;
  for (int i = 0; i < sizeof(typenames) / sizeof(*typenames); i++){
    if (consume(typenames[i])){
      token = saved;
      return true;
    }
  }
  if (consume("struct")){
    token = saved;
    return true;
  }
  Token *ident = consume_ident();
  if (ident){
    VarScope *vsc = find_var(ident);
    if (vsc && vsc->type_def){
      Type *type = consume_pointer(vsc->type_def);
      token = saved;
      return type;
    }
  }

  token = saved;
  return false;
}
