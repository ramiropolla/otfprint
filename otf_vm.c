#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_mem.h"
#include "otf_cff.h"
#include "otf_vm.h"
#include "otf_read.h"

struct stack *vm_stack_new(void)
{
	struct stack *r = x_calloc(1, sizeof(struct stack));

	return r;
}
void vm_stack_push(struct stack *stack, struct cff_operax *op)
{
	stack->operands[stack->idx++] = op;
	if (stack->idx >= 48) {
		fprintf(stderr, "too many stack pushes\n");
		exit(-1);
	}
}
struct cff_operax *vm_stack_pop(struct stack *stack)
{
	if (stack->idx == 0) {
		fprintf(stderr, "too many stack pops\n");
		exit(-1);
	}
	struct cff_operax * r = stack->operands[--stack->idx];
	return r;
}
void vm_stack_clear(struct stack *stack)
{
	while (vm_stack_idx(stack)) {
		struct cff_operax *op = vm_stack_pop(stack);
		free(op);
	}
}
int vm_stack_idx(struct stack *stack)
{
	return stack->idx;
}

struct vm *otf_vm_new(struct cff *cff, struct font *font)
{
	struct vm *vm = x_calloc(1, sizeof(struct vm));

	vm->stack = vm_stack_new();
	vm->font = font;
	vm->cff = cff;

	return vm;
}
void otf_vm_free(struct vm *vm)
{
	free(vm->stack);
	free(vm);
}

void vm_check_width(struct vm *vm)
{
	if (!vm->width_set) {
		struct cff_operax *op = vm_stack_pop(vm->stack);
		vm->width = op->v;
		vm->width_set = 1;
		printf("\twidth(%d);\n", vm->width);
		free(op);
	}
}

static void callgsubr(struct vm *vm, int level)
{
	struct cff_index_data *global_subr_idx = vm->cff->global_subr_idx;
	struct cff_operax *op = vm_stack_pop(vm->stack);
	int subr_n = vm->bias + op->v;
	uint8_t *p = global_subr_idx->data + global_subr_idx->offset[subr_n] - 1;
	int size = global_subr_idx->offset[subr_n+1] - global_subr_idx->offset[subr_n] + 1;

	otf_vm_go(vm, level+1, &p, size);

	free(op);
}
static void rmoveto(struct vm *vm)
{
	struct cff_operax *dx, *dy;

	vm_check_width(vm);

	dy = vm_stack_pop(vm->stack);
	dx = vm_stack_pop(vm->stack);

	printf("\trmoveto(%d, %d);\n", dx->v, dy->v);

	free(dx);
	free(dy);

	vm_stack_clear(vm->stack);
}
static void vhcurveto(struct vm *vm)
{
	int idx = vm_stack_idx(vm->stack);

	if        (idx ==  9) {
#if 0
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
		struct cff_operax *dyf = vm_stack_pop(vm->stack);
#endif
	} else if (idx == 13) {
	} else {
		printf("\trvhcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	vm_stack_clear(vm->stack);
}

void otf_vm_operate(struct vm *vm, int level, struct cff_operax *op)
{
//	fprintf(stderr, "%s(%p, %d, [%d,%d,%d]);\n", __func__, vm, level, op->type, op->flags, op->v);

	if (op->flags & ESCAPED) {
		switch (op->v) {
		default: goto not_impl;
		}
	} else {
		switch (op->v) {
		case 21: rmoveto(vm); break;
		case 29: callgsubr(vm, level); break;
		case 30: vhcurveto(vm); break;
		default: goto not_impl;
		}
	}

//	fprintf(stderr, "%s end\n", __func__);
	return;

not_impl:
	fprintf(stderr, "%s: %c%d not implemented\n", __func__, op->flags & ESCAPED ? 'x' : ' ', op->v);
	exit(-1);
}
void otf_vm_go(struct vm *vm, int level, uint8_t **pp, int size)
{
	uint8_t *p = *pp;

//	fprintf(stderr, "%s(%p, %d, %p, %d);\n", __func__, vm, level, p, size);

	while (p <= p + size) {
		struct cff_operax *op;

		while (1) {
			op = cff_type2_parse_operax(&p);
			if (op->type != OPERAX_OPERAND)
				break;
			vm_stack_push(vm->stack, op);
		}

		otf_vm_operate(vm, level, op);
	}

//	fprintf(stderr, "%s end (%d)\n", __func__, level);

	*pp = p;
	return;
}

int otf_vm(struct cff *cff, struct font *font, int gid)
{
	struct vm *vm = otf_vm_new(cff, font);
	uint32_t glyph = font->charset_data->glyph[gid];

	vm->bias = cff->global_subr_idx->count;

	printf("*** OTF_VM\n");
	printf("glyph: %d\n", glyph);

	printf("cs count: %d\n", font->CharStrings_idx->count);

	uint8_t *cs_p    = font->CharStrings_idx->data            + font->CharStrings_idx->offset[glyph] - 1;
	int      cs_size = font->CharStrings_idx->offset[glyph+1] - font->CharStrings_idx->offset[glyph] + 1;

	otf_vm_go(vm, 0, &cs_p, cs_size);

	otf_vm_free(vm);

	return 0;
}
