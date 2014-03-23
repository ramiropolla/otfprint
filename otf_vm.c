#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "otf_mem.h"
#include "otf_cff.h"
#include "otf_vm.h"
#include "otf_read.h"

#define SVG_OUT
//#define FUNC_IN printf(">>%s(%d)\n", __func__, vm_stack_idx(vm->stack));

//#define CALLSUB
#ifndef FUNC_IN
#define FUNC_IN
#endif

struct xy {
	int x;
	int y;
};
struct bezier {
	struct xy a;
	struct xy b;
	struct xy c;
};
static void path_double(struct vm *vm)
{
	int newsize = vm->path_size << 1;

	vm->path = x_realloc(vm->path, newsize);

	vm->path_left += vm->path_size;
	vm->path_size = newsize;
}
static void path_append(struct vm *vm, char *str, int len)
{
	while (vm->path_left <= len)
		path_double(vm);
	memcpy(vm->path + vm->path_offset, str, len);
	vm->path_offset += len;
	vm->path_left -= len;
	vm->path[vm->path_offset] = 0;
}
static void path_printf(struct vm *vm, const char *fmt, ...)
{
	char *str = NULL;
	int len;

	va_list ap, apx;
	va_start(ap, fmt);
	va_copy(apx, ap);

	len = vsnprintf(str, 0, fmt, ap);
	str = x_malloc(len+1);
	vsnprintf(str, len+1, fmt, apx);
	path_append(vm, str, len);
	free(str);

	va_end(apx);
	va_end(ap);
}
static void print_bezier(struct vm *vm, struct bezier *b)
{
#ifdef SVG_OUT
	path_printf(vm, "c%d %d %d %d %d %d",
	            b->a.x, b->a.y,
	            b->b.x + b->a.x, b->b.y + b->a.y,
	            b->c.x + b->b.x + b->a.x, b->c.y + b->b.y + b->a.y);
#else
	printf("\tbezier[%d,%d %d,%d %d,%d]\n",
	       b->a.x, b->a.y,
	       b->b.x + b->a.x, b->b.y + b->a.y,
	       b->c.x + b->b.x + b->a.x, b->c.y + b->b.y + b->a.y);
//	printf("\tbezier[%d,%d %d,%d %d,%d]\n", b->a.x, b->a.y, b->b.x, b->b.y, b->c.x, b->c.y);
#endif
}
static void print_line(struct vm *vm, int dx, int dy)
{
#ifdef SVG_OUT
	if      (!dx) path_printf(vm, "v%d", dy);
	else if (!dy) path_printf(vm, "h%d", dx);
	else          path_printf(vm, "l%d %d", dx, dy);
#else
	printf("\tline[%d,%d]\n", dx, dy);
#endif
}
static void print_move(struct vm *vm, int dx, int dy)
{
#ifdef SVG_OUT
	if (vm->path[0])
		path_printf(vm, "m%d %d", dx, dy);
	else
		path_printf(vm, "M%d %d", dx, dy);
#else
	printf("\tmove[%d, %d]\n", dx, dy);
#endif
}

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

	vm->width = vm->font->private_data->defaultWidthX;

	vm->path_size = 32;
	vm->path_left = vm->path_size;
	path_double(vm);
	vm->path[0] = 0;

	return vm;
}
void otf_vm_free(struct vm *vm)
{
	free(vm->stack);
	free(vm->path);
	free(vm);
}

int vm_check_width(struct vm *vm, int op_idx)
{
	int idx = vm_stack_idx(vm->stack);
	if (!vm->width_set && idx == (op_idx+1)) {
		vm->width = vm->font->private_data->nominalWidthX
		          + vm_stack_peek_value(vm->stack, 0);
		vm->width_set = 1;
		return 1;
	}
	return 0;
}

static int callgsubr(struct vm *vm, int level)
{
	FUNC_IN
	struct cff_index_data *global_subr_idx = vm->cff->global_subr_idx;
	struct cff_operax *op = vm_stack_pop(vm->stack);
	int subr_n = vm->gbias + op->v;
	uint8_t *p = global_subr_idx->data + global_subr_idx->offset[subr_n] - 1;
	int size = global_subr_idx->offset[subr_n+1] - global_subr_idx->offset[subr_n] + 1;
	int r;

#ifdef CALLSUB
	printf("\tcallgsubr(%p, %d, subr_n %d);\n", p, size, subr_n);
#endif
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

#ifdef CALLSUB
	printf("\tcallsubr(%p, %d, subr_n %d);\n", p, size, subr_n);
#endif
	r = otf_vm_go(vm, level+1, &p, size);

	free(op);

	return r;
}
static void rmoveto(struct vm *vm)
{
	FUNC_IN
	int dx, dy;
	int offset = vm_check_width(vm, 2);
	int idx = vm_stack_idx(vm->stack);

	if (idx - offset != 2) {
		printf("\trmoveto with %d items on stack\n", idx - offset);
		exit(-1);
	}

	dx = vm_stack_peek_value(vm->stack, 0 + offset);
	dy = vm_stack_peek_value(vm->stack, 1 + offset);

	print_move(vm, dx, dy);

	vm_stack_clear(vm->stack);
}
static void vmoveto(struct vm *vm)
{
	FUNC_IN
	int dy1;
	int offset = vm_check_width(vm, 1);
	int idx = vm_stack_idx(vm->stack);

	if (idx - offset != 1) {
		printf("\tvmoveto with %d items on stack\n", idx - offset);
		exit(-1);
	}

	dy1 = vm_stack_peek_value(vm->stack, 0 + offset);

	print_move(vm, 0, dy1);

	vm_stack_clear(vm->stack);
}
static void hmoveto(struct vm *vm)
{
	FUNC_IN
	int dx1;
	int offset = vm_check_width(vm, 1);
	int idx = vm_stack_idx(vm->stack);

	if (idx - offset != 1) {
		printf("\thmoveto with %d items on stack\n", idx - offset);
		exit(-1);
	}

	dx1 = vm_stack_peek_value(vm->stack, 0 + offset);

	print_move(vm, dx1, 0);

	vm_stack_clear(vm->stack);
}
static struct bezier bezier_parse_full(struct vm *vm, int offset)
{
	struct bezier b;
	b.a.x = vm_stack_peek_value(vm->stack, 0 + offset);
	b.a.y = vm_stack_peek_value(vm->stack, 1 + offset);
	b.b.x = vm_stack_peek_value(vm->stack, 2 + offset);
	b.b.y = vm_stack_peek_value(vm->stack, 3 + offset);
	b.c.x = vm_stack_peek_value(vm->stack, 4 + offset);
	b.c.y = vm_stack_peek_value(vm->stack, 5 + offset);
	return b;
}
static struct bezier bezier_parse_v(struct vm *vm, int offset)
{
	struct bezier b;
	b.a.x = 0;
	b.a.y = vm_stack_peek_value(vm->stack, 0 + offset);
	b.b.x = vm_stack_peek_value(vm->stack, 1 + offset);
	b.b.y = vm_stack_peek_value(vm->stack, 2 + offset);
	b.c.x = vm_stack_peek_value(vm->stack, 3 + offset);
	b.c.y = 0;
	return b;
}
static struct bezier bezier_parse_h(struct vm *vm, int offset)
{
	struct bezier b;
	b.a.x = vm_stack_peek_value(vm->stack, 0 + offset);
	b.a.y = 0;
	b.b.x = vm_stack_peek_value(vm->stack, 1 + offset);
	b.b.y = vm_stack_peek_value(vm->stack, 2 + offset);
	b.c.x = 0;
	b.c.y = vm_stack_peek_value(vm->stack, 3 + offset);
	return b;
}
static void vhcurveto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);

	if (((idx%8)&~1) == 4) { /* 4, 5, 12, 13, 20, 21, ... */
		struct bezier b = bezier_parse_v(vm, 0);
		int offset = 0;
		if (idx == 5)
			b.c.y = vm_stack_peek_value(vm->stack, 4);
		print_bezier(vm, &b);
		offset += 4;
		idx -= 4;

		while (idx >= 8) {
			struct bezier b1 = bezier_parse_h(vm, offset);
			struct bezier b2 = bezier_parse_v(vm, offset + 4);
			if (idx == 9)
				b2.c.y = vm_stack_peek_value(vm->stack, 8 + offset);
			print_bezier(vm, &b1);
			print_bezier(vm, &b2);
			offset += 8;
			idx -= 8;
		}
	} else if (!((idx%8)&~1)) { /* 8, 9, 16, 17, ... */
		int offset = 0;
		while (idx >= 8) {
			struct bezier b1 = bezier_parse_v(vm, offset);
			struct bezier b2 = bezier_parse_h(vm, offset + 4);
			if (idx == 9)
				b2.c.x = vm_stack_peek_value(vm->stack, 8 + offset);
			print_bezier(vm, &b1);
			print_bezier(vm, &b2);
			offset += 8;
			idx -= 8;
		}
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

	if (((idx%8)&~1) == 4) { /* 4, 5, 12, 13, 20, 21, ... */
		struct bezier b = bezier_parse_h(vm, 0);
		int offset = 0;
		if (idx == 5)
			b.c.x = vm_stack_peek_value(vm->stack, 4);
		print_bezier(vm, &b);
		offset += 4;
		idx -= 4;

		while (idx >= 8) {
			struct bezier b1 = bezier_parse_v(vm, offset);
			struct bezier b2 = bezier_parse_h(vm, offset + 4);
			if (idx == 9)
				b2.c.x = vm_stack_peek_value(vm->stack, 8 + offset);
			print_bezier(vm, &b1);
			print_bezier(vm, &b2);
			offset += 8;
			idx -= 8;
		}
	} else if (!((idx%8)&~1)) { /* 8, 9, 16, 17, ... */
		int offset = 0;
		while (idx >= 8) {
			struct bezier b1 = bezier_parse_h(vm, offset);
			struct bezier b2 = bezier_parse_v(vm, offset + 4);
			if (idx == 9)
				b2.c.y = vm_stack_peek_value(vm->stack, 8 + offset);
			print_bezier(vm, &b1);
			print_bezier(vm, &b2);
			offset += 8;
			idx -= 8;
		}
	} else {
		printf("\thvcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	vm_stack_clear(vm->stack);
}
static void rrcurveto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;

	if (idx%6) {
		printf("\trrcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	while (idx >= 6) {
		struct bezier b = bezier_parse_full(vm, offset);
		print_bezier(vm, &b);
		offset += 6;
		idx -= 6;
	}

	vm_stack_clear(vm->stack);
}
static void rcurveline(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;
	int dx, dy;

	if (idx < 8 || (idx%6) != 2) {
		printf("\trcurveline with %d items on stack\n", idx);
		exit(-1);
	}

	while (idx > 6) {
		struct bezier b = bezier_parse_full(vm, offset);
		print_bezier(vm, &b);
		offset += 6;
		idx -= 6;
	}
	dx = vm_stack_peek_value(vm->stack, 0 + offset);
	dy = vm_stack_peek_value(vm->stack, 1 + offset);
	print_line(vm, dx, dy);

	vm_stack_clear(vm->stack);
}
static void rlinecurve(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;
	struct bezier b;

	if (idx < 8 || idx&1) {
		printf("\trlinecurve with %d items on stack\n", idx);
		exit(-1);
	}
	while (idx > 6) {
		int dx, dy;
		dx = vm_stack_peek_value(vm->stack, 0 + offset);
		dy = vm_stack_peek_value(vm->stack, 1 + offset);
		print_line(vm, dx, dy);
		offset += 2;
		idx -= 2;
	}
	b = bezier_parse_full(vm, offset);
	print_bezier(vm, &b);

	vm_stack_clear(vm->stack);
}
static void hhcurveto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;

	if (idx < 4 || ((idx%4)&~1)) { /* 4, 5, 8, 9, ... */
		printf("\thhcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	while (idx >= 4) {
		struct bezier b;
		b.a.y = 0;
		if (idx&1) {
			b.a.y = vm_stack_peek_value(vm->stack, 0);
			offset++;
			idx--;
		}
		b.a.x = vm_stack_peek_value(vm->stack, 0 + offset);
		b.b.x = vm_stack_peek_value(vm->stack, 1 + offset);
		b.b.y = vm_stack_peek_value(vm->stack, 2 + offset);
		b.c.x = vm_stack_peek_value(vm->stack, 3 + offset);
		b.c.y = 0;
		print_bezier(vm, &b);
		offset += 4;
		idx -= 4;
	}

	vm_stack_clear(vm->stack);
}
static void vvcurveto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;

	if (idx < 4 || ((idx%4)&~1)) { /* 4, 5, 8, 9, ... */
		printf("\tvvcurveto with %d items on stack\n", idx);
		exit(-1);
	}

	while (idx >= 4) {
		struct bezier b;
		b.a.x = 0;
		if (idx&1) {
			b.a.x = vm_stack_peek_value(vm->stack, 0);
			offset++;
			idx--;
		}
		b.a.y = vm_stack_peek_value(vm->stack, 0 + offset);
		b.b.x = vm_stack_peek_value(vm->stack, 1 + offset);
		b.b.y = vm_stack_peek_value(vm->stack, 2 + offset);
		b.c.x = 0;
		b.c.y = vm_stack_peek_value(vm->stack, 3 + offset);
		print_bezier(vm, &b);
		offset += 4;
		idx -= 4;
	}

	vm_stack_clear(vm->stack);
}
static void vlineto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;

	while (idx--) {
		int val = vm_stack_peek_value(vm->stack, offset++);
		if (offset&1) {
			print_line(vm, 0, val);
		} else {
			print_line(vm, val, 0);
		}
	}

	vm_stack_clear(vm->stack);
}
static void hlineto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;

	while (idx--) {
		int val = vm_stack_peek_value(vm->stack, offset++);
		if (offset&1) {
			print_line(vm, val, 0);
		} else {
			print_line(vm, 0, val);
		}
	}

	vm_stack_clear(vm->stack);
}
static void rlineto(struct vm *vm)
{
	FUNC_IN
	int idx = vm_stack_idx(vm->stack);
	int offset = 0;

	if (idx&1) {
		printf("\trlineto with %d items on stack\n", idx);
		exit(-1);
	}

	while (idx >= 2) {
		int dx = vm_stack_peek_value(vm->stack, 0 + offset);
		int dy = vm_stack_peek_value(vm->stack, 1 + offset);
		print_line(vm, dx, dy);
		offset += 2;
		idx -= 2;
	}

	vm_stack_clear(vm->stack);
}
static void endchar(struct vm *vm)
{
	vm_check_width(vm, 0);
#ifdef SVG_OUT
	path_printf(vm, "z");
#else
	printf("\tendchar\n");
#endif
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
		case  5: rlineto(vm);          break;
		case  6: hlineto(vm);          break;
		case  7: vlineto(vm);          break;
		case  8: rrcurveto(vm);        break;
		case 10: if (callsubr(vm, level) == 2) r = 2; break;
		case 11: /* printf(">>return\n"); */ r = 1; break;
		case 14: endchar(vm); r = 2;   break;
		case 21: rmoveto(vm);          break;
		case 22: hmoveto(vm);          break;
		case 24: rcurveline(vm);       break;
		case 25: rlinecurve(vm);       break;
		case 26: vvcurveto(vm);        break;
		case 27: hhcurveto(vm);        break;
		case 29: if (callgsubr(vm, level) == 2) r = 2; break;
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
	uint8_t *p0 = p;
	int r = 0;

//	printf("%s(%p, %d, %p, %d);\n", __func__, vm, level, p, size);

	while (p <= p0 + size) {
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
struct glyph *otf_vm(struct cff *cff, struct font *font, int gid)
{
	struct vm *vm = otf_vm_new(cff, font);
	struct glyph *glyph;

	gid++;

	vm->gbias = subr_bias(cff->global_subr_idx->count);
	if (font->local_subr_idx)
		vm->lbias = subr_bias(font->local_subr_idx->count);

	uint8_t *cs_p    = font->CharStrings_idx->data          + font->CharStrings_idx->offset[gid] - 1;
	int      cs_size = font->CharStrings_idx->offset[gid+1] - font->CharStrings_idx->offset[gid] + 1;

	otf_vm_go(vm, 0, &cs_p, cs_size);

	glyph = x_malloc(sizeof(struct glyph));
	glyph->path = strdup(vm->path);
	glyph->width = vm->width;

	otf_vm_free(vm);

	return glyph;
}
