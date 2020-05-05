#include "tcc.h"

// Edit here to add type
// - Must be in order with TypeKind declaration.
// - Must be same size as declaration.
char *typenames[] = {
  "char", "int",
};

int get_size(Type *type){
  switch (type->kind){
    case TY_CHAR:
      return 1;
    case TY_INT:
      return 8;
    case TY_PTR:
      return 8;
    case TY_ARRAY:
      return type->size * type->array_len;
    case TY_STRUCT:
      return 0;
    default:
      error("Undefined type %d in get_size()", type->kind);
  }
}

int get_align(Type *type){
  switch (type->kind){
    case TY_CHAR:
      return 1;
    case TY_INT:
      return 8;
    case TY_PTR:
      return 8;
    case TY_ARRAY:
      if (type->ptr_to){
        return type->ptr_to->align;
      }
      return 0;
    case TY_STRUCT:
      return 0;
    default:
      error("Undefined type %d in get_aligh()", type->kind);
  }
}

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
  Type *st = new_type(TY_STRUCT, align_to(offset, st->align), align);
  st->members = members;

  return st;
}

Type *new_int_type(){
  return new_type(TY_INT, 8, 8);
}

Type *new_char_type(){
  return new_type(TY_INT, 1, 1);
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

  Member *member = calloc(1, sizeof(Member));
  member->type = type;
  member->name = name;

  expect(";");
  return member;
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
