#include "tcc.h"

// Must be in order with TypeKind
// Must edit tcc.h
char *typenames[] = {
  "char", "int",
};

int get_size(Type *type){
  switch (type->kind){
    case TY_CHAR:
      return 1;
      break;
    case TY_INT:
      return 8;
      break;
    case TY_PTR:
      return 8;
      break;
    case TY_ARRAY:
      return type->size * type->array_len;
      break;
    case TY_STRUCT:
      break;
    default:
      error("Undefined type %d in new_type", type->kind);
  }
}

Type *new_type(TypeKind kind){
  Type *type = calloc(1, sizeof(Type));
  type->kind = kind;
  type->size = get_size(type);
  return type;
}

Type *new_array_type(Type *ptr_to, int length){
  Type *arr = new_type(TY_ARRAY);
  arr->array_len = length;
  arr->ptr_to = ptr_to;
  arr->size = get_size(arr);

  return arr;
}

Type *new_struct_type(Member *members){
  Type *st = new_type(TY_STRUCT);
  st->members = members;

  // calc offset & size
  int offset = 0;
  for (Member *mem = members; mem; mem = mem->next){
    mem->offset = offset;
    offset += mem->type->size;
  }
  st->size = offset;

  return st;
}

Type *pointer_to(Type *type){
  Type *ty = new_type(TY_PTR);
  ty->ptr_to = type;
  return ty;
}

Member *struct_member(){
  Type *type = expect_type();
  type = consume_pointer(type);
  char *name = expect_ident();

  Member *member = calloc(1, sizeof(Member));
  member->type = type;
  member->name = name;

  expect(";");
  return member;
}

Type *expect_type(){
  for (int i = 0; i < sizeof(typenames) / sizeof(*typenames); i++){
    if (consume(typenames[i])){
      Type *ty = new_type(i);
      ty = consume_pointer(ty);
      return ty;
    }
  }
  if (consume("struct")){
    expect("{");
    Member head = {};
    Member *cur = &head;
    while (! consume("}")){
      cur->next = struct_member();
      cur = cur->next;
    }
    // Type *ty = new_type(TY_STRUCT);
    // ty->members = head.next;
    Member *members = head.next;
    Type *ty = new_struct_type(members);

    return ty;
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
    token = saved;
  }
  saved = token;
  if (consume("struct")){
    token = saved;
    return true;
  }

  return false;
}
