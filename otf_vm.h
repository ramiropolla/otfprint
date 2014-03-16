struct stack {
	struct cff_operax *operands[48];
	int idx;
};
struct stack *     vm_stack_new(void);
void               vm_stack_push(struct stack *stack, struct cff_operax *op);
struct cff_operax *vm_stack_pop(struct stack *stack);
int                vm_stack_idx(struct stack *stack);
void               vm_stack_clear(struct stack *stack);

struct vm {
	struct stack *stack;
	struct font *font;
	struct cff *cff;
	int bias;

	int width_set;
	int width;
};

int otf_vm(struct cff *cff, struct font *font, uint8_t c);
struct vm *otf_vm_new(struct cff *cff, struct font *font);
void otf_vm_free(struct vm *vm);
void otf_vm_operate(struct vm *vm, int level, struct cff_operax *op);
void otf_vm_go(struct vm *vm, int level, uint8_t **pp, int size);
