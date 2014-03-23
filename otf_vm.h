#ifndef OTF_VM_H
#define OTF_VM_H

struct stack {
	struct cff_operax *operands[48];
	int idx;
};
struct stack *     vm_stack_new(void);
void               vm_stack_push(struct stack *stack, struct cff_operax *op);
struct cff_operax *vm_stack_pop(struct stack *stack);
struct cff_operax *vm_stack_peek(struct stack *stack, int idx);
int                vm_stack_peek_value(struct stack *stack, int idx);
int                vm_stack_idx(struct stack *stack);
void               vm_stack_clear(struct stack *stack);

struct vm {
	struct stack *stack;
	struct font *font;
	struct cff *cff;
	int gbias;
	int lbias;

	int width_set;
	int width;

	char *path;
	int path_size;
	int path_left;
	int path_offset;
};

struct glyph {
	char *path;
	int width;
	uint16_t c;
};

struct glyph *otf_vm(struct cff *cff, struct font *font, int gid);
struct vm *otf_vm_new(struct cff *cff, struct font *font);
void otf_vm_free(struct vm *vm);
int otf_vm_operate(struct vm *vm, int level, struct cff_operax *op);
int otf_vm_go(struct vm *vm, int level, uint8_t **pp, int size);

#endif /* OTF_VM_H */
