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
//	printf("%s(%d)\n", __func__, op->v);
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
//	printf("%s: %d\n", __func__, r->v);
	return r;
}
struct cff_operax *vm_stack_peek(struct stack *stack, int idx)
{
	return stack->operands[idx];
}
int vm_stack_peek_value(struct stack *stack, int idx)
{
	return vm_stack_peek(stack, idx)->v;
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

//#define FUNC_IN printf(">>%s(%d)\n", __func__, vm_stack_idx(vm->stack));
#define FUNC_IN

static int callgsubr(struct vm *vm, int level)
{
	FUNC_IN
	struct cff_index_data *global_subr_idx = vm->cff->global_subr_idx;
	struct cff_operax *op = vm_stack_pop(vm->stack);
	int subr_n = vm->gbias + op->v;
	uint8_t *p = global_subr_idx->data + global_subr_idx->offset[subr_n] - 1;
	int size = global_subr_idx->offset[subr_n+1] - global_subr_idx->offset[subr_n] + 1;
	int r;

//	printf("\tcallgsubr(%p, %d, subr_n %d);\n", p, size, subr_n);
	r = otf_vm_go(vm, level+1, &p, size);

	free(op);

	return r;
}
static int callsubr(struct vm *vm, int level)
{
	FUNC_IN
	struct cff_index_data *local_subr_idx = vm->font->local_subr_idx;
	struct cff_operax *op = vm_stack_pop(vm->stack);
	int subr_n = vm->lbias + op->v;
	uint8_t *p = local_subr_idx->data + local_subr_idx->offset[subr_n] - 1;
	int size = local_subr_idx->offset[subr_n+1] - local_subr_idx->offset[subr_n] + 1;
	int r;

//	printf("\tcallsubr(%p, %d, subr_n %d);\n", p, size, subr_n);
	r = otf_vm_go(vm, level+1, &p, size);

	free(op);

	return r;
}
static void rmoveto(struct vm *vm)
{
	FUNC_IN
	int dx, dy;

	vm_check_width(vm);

// 	printf("rmoveto (%d)\n", vm_stack_idx(vm->stack));

	dx = vm_stack_peek_value(vm->stack, 0);
	dy = vm_stack_peek_value(vm->stack, 1);

	printf("\trmoveto(%d, %d);\n", dx, dy);

	vm_stack_clear(vm->stack);
}
static void vmoveto(struct vm *vm)
{
	FUNC_IN
	int dy1;

	vm_check_width(vm);

// 	printf("vmoveto (%d)\n", vm_stack_idx(vm->stack));

	dy1 = vm_stack_peek_value(vm->stack, 0);

	printf("\tvmoveto(%d);\n", dy1);

	vm_stack_clear(vm->stack);
}
struct xy {
	int x;
	int y;
};
struct bezier {
	struct xy a;
	struct xy b;
	struct xy c;
};
static void vhcurveto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);

	if        (idx ==  8 || idx ==  9) {
		struct bezier b1, b2;
		b1.a.x = 0;
		b1.a.y = vm_stack_peek_value(vm->stack, 0);
		b1.b.x = vm_stack_peek_value(vm->stack, 1);
		b1.b.y = vm_stack_peek_value(vm->stack, 2);
		b1.c.x = vm_stack_peek_value(vm->stack, 3);
		b1.c.y = 0;
		b2.a.x = vm_stack_peek_value(vm->stack, 4);
		b2.a.y = 0;
		b2.b.x = vm_stack_peek_value(vm->stack, 5);
		b2.b.y = vm_stack_peek_value(vm->stack, 6);
		b2.c.x = 0;
		b2.c.y = vm_stack_peek_value(vm->stack, 7);
		if (idx == 9)
			b2.c.x = vm_stack_peek_value(vm->stack, 8);
		printf("\tvhcurveto: [%d,%d %d,%d %d,%d] [%d,%d %d,%d %d,%d]\n", b1.a.x, b1.a.y, b1.b.x, b1.b.y, b1.c.x, b1.c.y, b2.a.x, b2.a.y, b2.b.x, b2.b.y, b2.c.x, b2.c.y);
	} else {
		printf("\tvhcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	vm_stack_clear(vm->stack);
}
static void hvcurveto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);

	if        (idx == 4 || idx == 5) {
		int dx1 = vm_stack_peek_value(vm->stack, 0);
		int dx2 = vm_stack_peek_value(vm->stack, 1);
		int dy1 = vm_stack_peek_value(vm->stack, 2);
		int dy2 = vm_stack_peek_value(vm->stack, 3);
		printf("\thhcurveto: [%d,%d %d,%d]", dx1, dy1, dx2, dy2);
		if (idx == 5)
			printf(" %d", vm_stack_peek_value(vm->stack, 4));
		printf("\n");
	} else if (idx ==  8 || idx == 9 || idx == 16) {
		while (idx >= 8) {
			struct bezier b1, b2;
			b1.a.x = vm_stack_peek_value(vm->stack, 0);
			b1.a.y = 0;
			b1.b.x = vm_stack_peek_value(vm->stack, 1);
			b1.b.y = vm_stack_peek_value(vm->stack, 2);
			b1.c.x = 0;
			b1.c.y = vm_stack_peek_value(vm->stack, 3);
			b2.a.x = 0;
			b2.a.y = vm_stack_peek_value(vm->stack, 4);
			b2.b.x = vm_stack_peek_value(vm->stack, 5);
			b2.b.y = vm_stack_peek_value(vm->stack, 6);
			b2.c.x = vm_stack_peek_value(vm->stack, 7);
			b2.c.y = 0;
			if (idx == 9)
				b2.c.y = vm_stack_peek_value(vm->stack, 8);
			printf("\thvcurveto: [%d,%d %d,%d %d,%d] [%d,%d %d,%d %d,%d]\n", b1.a.x, b1.a.y, b1.b.x, b1.b.y, b1.c.x, b1.c.y, b2.a.x, b2.a.y, b2.b.x, b2.b.y, b2.c.x, b2.c.y);
			idx -= 8;
		}
	} else if (idx == 12 || idx == 13) {
		int dx1 = vm_stack_peek_value(vm->stack, 0);
		int dx2 = vm_stack_peek_value(vm->stack, 1);
		int dy2 = vm_stack_peek_value(vm->stack, 2);
		int dy3 = vm_stack_peek_value(vm->stack, 3);
		struct bezier b1, b2;
		b1.a.x = 0;
		b1.a.y = vm_stack_peek_value(vm->stack, 4);
		b1.b.x = vm_stack_peek_value(vm->stack, 5);
		b1.b.y = vm_stack_peek_value(vm->stack, 6);
		b1.c.x = vm_stack_peek_value(vm->stack, 7);
		b1.c.y = 0;
		b2.a.x = vm_stack_peek_value(vm->stack, 8);
		b2.a.y = 0;
		b2.b.x = vm_stack_peek_value(vm->stack, 9);
		b2.b.y = vm_stack_peek_value(vm->stack, 10);
		b2.c.x = 0;
		b2.c.y = vm_stack_peek_value(vm->stack, 11);
		if (idx == 13)
			b2.c.x = vm_stack_peek_value(vm->stack, 12);
		printf("\thvcurveto: %d %d %d %d [%d,%d %d,%d %d,%d] [%d,%d %d,%d %d,%d]\n", dx1, dx2, dy2, dy3, b1.a.x, b1.a.y, b1.b.x, b1.b.y, b1.c.x, b1.c.y, b2.a.x, b2.a.y, b2.b.x, b2.b.y, b2.c.x, b2.c.y);
	} else {
		printf("\thvcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	vm_stack_clear(vm->stack);
}
static void hhcurveto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);

	if        (idx ==  5) {
		struct bezier b;
		int y1 = vm_stack_peek_value(vm->stack, 0);
		b.a.x = vm_stack_peek_value(vm->stack, 1);
		b.a.y = 0;
		b.b.x = vm_stack_peek_value(vm->stack, 2);
		b.b.y = vm_stack_peek_value(vm->stack, 3);
		b.c.x = vm_stack_peek_value(vm->stack, 4);
		b.c.y = 0;
		printf("\thhcurveto: %d [%d,%d %d,%d %d,%d]\n", y1, b.a.x, b.a.y, b.b.x, b.b.y, b.c.x, b.c.y);
	} else {
		printf("\thhcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	vm_stack_clear(vm->stack);
}
static void vlineto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);

	if (idx == 1) {
		int dy1 = vm_stack_peek_value(vm->stack, 0);
		printf("\tvlineto: %d\n", dy1);
	} else {
		printf("\tvlineto with %d items on stack\n", idx);
		exit(-1);
	}

	vm_stack_clear(vm->stack);
}

int otf_vm_operate(struct vm *vm, int level, struct cff_operax *op)
{
//	printf("%s(%p, %d, [%d,%d,%d]);\n", __func__, vm, level, op->type, op->flags, op->v);
	int r = 0;

	if (op->flags & ESCAPED) {
		switch (op->v) {
		default: goto not_impl;
		}
	} else {
		switch (op->v) {
		case  4: vmoveto(vm);          break;
		case  7: vlineto(vm);          break;
		case 10: if (callsubr(vm, level) == 2) r = 1; break;
		case 11: /* printf(">>return\n"); */ r = 1; break;
		case 14: /* printf(">>endchar\n"); */ r = 2; break;
		case 21: rmoveto(vm);          break;
		case 27: hhcurveto(vm);        break;
		case 29: if (callgsubr(vm, level) == 2) r = 1; break;
		case 30: vhcurveto(vm);        break;
		case 31: hvcurveto(vm);        break;
		default: goto not_impl;
		}
	}

//	printf("%s end\n", __func__);
	return r;

not_impl:
	fprintf(stderr, "%s: %c%d not implemented\n", __func__, op->flags & ESCAPED ? 'x' : ' ', op->v);
	exit(-1);
	return 0;
}
int otf_vm_go(struct vm *vm, int level, uint8_t **pp, int size)
{
	uint8_t *p = *pp;
	int r = 0;

//	printf("%s(%p, %d, %p, %d);\n", __func__, vm, level, p, size);

	while (p <= p + size) {
		struct cff_operax *op;

		while (1) {
			op = cff_type2_parse_operax(&p);
			if (op->type != OPERAX_OPERAND)
				break;
			vm_stack_push(vm->stack, op);
		}

		r = otf_vm_operate(vm, level, op);
		if (r)
			break;
	}

//	printf("%s end (%d)\n", __func__, level);

	*pp = p;
	return r;
}

static int subr_bias(int n)
{
	if (n < 1240)
		return 107;
	else if (n < 33900)
		return 1131;
	return 32768;
}
int otf_vm(struct cff *cff, struct font *font, int gid)
{
	struct vm *vm = otf_vm_new(cff, font);
	uint32_t glyph = font->charset_data->glyph[gid];

	vm->gbias = subr_bias(cff->global_subr_idx->count);
	vm->lbias = subr_bias(font->local_subr_idx->count);

	printf("*** OTF_VM\n");
	printf("glyph: %d\n", glyph);

	printf("cs count: %d\n", font->CharStrings_idx->count);

	uint8_t *cs_p    = font->CharStrings_idx->data            + font->CharStrings_idx->offset[glyph] - 1;
	int      cs_size = font->CharStrings_idx->offset[glyph+1] - font->CharStrings_idx->offset[glyph] + 1;

	otf_vm_go(vm, 0, &cs_p, cs_size);

	otf_vm_free(vm);

	return 0;
}
