/*
 * By Yueqi Chen (yueqichen.0x0@gmail.com)
 */

#include "gcc-common.h"
#include "fengshui_plugin.h"
#include <assert.h>

const char* target_object_name = "pm_qos_request";
const char* subfield_type_array[] = {"seccomp_filter","bpf_prog"};
static int subfield_type_array_len = 2;

// PLUGIN INFO
__visible int plugin_is_GPL_compatible;

static struct plugin_info instrument_panic_plugin_info = {
	.version = "20190207",
	.help = "Instrument Panic check function into kernel\n",
};


// SLAB ALLOC FUNC
const static char* slab_alloc_func_array[] = {
// general
"__kmalloc", "__kmalloc_node", "kmalloc", "kmalloc_node", "kmalloc_array", 
"kzalloc", "kmalloc_array_node", "kzalloc_node", "kcalloc_node", "kcalloc",
// special
"kmem_cache_alloc", "kmem_cache_alloc_node", "kmem_cache_zalloc",
// corner
"sock_kmalloc"
};
const static int slab_alloc_func_array_len = 14;
bool is_slab_alloc_func(const char* func_name) {
	int i = 0;
	for (i = 0 ; i < slab_alloc_func_array_len ; i++) 
		if (strcmp(func_name, slab_alloc_func_array[i]) == 0)
			return true;
	return false;
}

// RCU FREE FUNC
const static char* rcu_free_func_array[] = {
"kfree_call_rcu", "call_rcu_sched"
};
const static int rcu_free_func_array_len = 2;
bool is_rcu_free_func(const char* func_name) {
	int i = 0;
	for (i = 0 ; i < rcu_free_func_array_len; i++)
		if (strcmp(func_name, rcu_free_func_array[i]) == 0)
			return true;
	return false;
}

// MAGIC WRONG NUM
const static long magic_wrong_num_array[] = {
0x900000000, 0x3100000000, 0x700000000, 0xa00000000, 0x1200000000,
0xe00000000, 0x1000000000, 0x600000000, 0x6f200000000, 0x5b000000000,
0x800000000
};
const static int magic_wrong_num_array_len = 11;
bool is_magic_wrong_num(const void* num) {
	int i = 0;
	for (i = 0; i < magic_wrong_num_array_len; i++)
		if ((long)num == magic_wrong_num_array[i])
			return true;
	return false;
}

#ifdef INSTRUMENT
// DECL PANIC FUNC
static tree proto1, proto2, proto3, proto4;
static tree decl1, decl2, decl3, decl4;
static void decl_panic_func(void* /*gcc data*/, void* /*user data*/)
{
#ifdef ALLOC_PLUGIN
	proto1 = build_function_type_list (
		void_type_node, // return type
		ptr_type_node, // first arg's type
		NULL_TREE);
	decl1 = build_fn_decl("panic_obj_alloc", proto1);
#endif

#ifdef DEREF_PLUGIN
	proto2 = build_function_type_list (
		void_type_node, // return type
		ptr_type_node, // first arg's type
		NULL_TREE);
	decl2 = build_fn_decl("panic_obj_dereference", proto2);
#endif 

#ifdef RCU_PLUGIN
	proto3 = build_function_type_list (
		void_type_node, // return type
		ptr_type_node, // first arg's type
		NULL_TREE);
	decl3 = build_fn_decl("panic_rcu_dereference", proto3);
#endif

#ifdef FREE_PLUGIN
	proto4 = build_function_type_list (
		void_type_node, // return type
		ptr_type_node, // first arg's type
		NULL_TREE);
	decl4 = build_fn_decl("panic_obj_free", proto4);
#endif
	return;
}

// DO INSTRUMENT
#define ALLOC_INSTRM 1
#define DEREF_INSTRM 2
#define RCU_INSTRM 3
#define FREE_INSTRM 4

static void instrument_panic(gimple stmt, int INSTRM_TYPE, const_tree obj_ptr) 
{
	switch(INSTRM_TYPE) {
		case ALLOC_INSTRM: 
			{
				const_tree lhs;
				const_tree lhs_type;
				lhs = gimple_get_lhs(stmt);
				assert(lhs != 0);
				lhs_type = TREE_TYPE(lhs);
				assert(TREE_CODE(lhs_type) == POINTER_TYPE);

				gimple call = gimple_build_call (decl1, 1, lhs);
				gimple_stmt_iterator gsi = gsi_for_stmt(stmt);
				gsi_insert_after(&gsi, call, GSI_NEW_STMT);
				break;
			}

		case DEREF_INSTRM:
			{
				// target->subfield
				const_tree rhs1 = gimple_assign_rhs1(stmt);
				// target
				const_tree rhs1_arg0 = TREE_OPERAND(rhs1, 0);
				// &target
				const_tree rhs1_arg0_arg0 = TREE_OPERAND(rhs1_arg0, 0);
				gimple call = gimple_build_call(decl2, 1, rhs1_arg0_arg0);
				gimple_stmt_iterator gsi = gsi_for_stmt(stmt);
				gsi_insert_after(&gsi, call, GSI_NEW_STMT);
				break;
			}

		case RCU_INSTRM:
			{
				gimple call = gimple_build_call(decl3, 1, obj_ptr);
				gimple_stmt_iterator gsi = gsi_for_stmt(stmt);
				gsi_insert_after(&gsi, call, GSI_NEW_STMT);
				break;
			}

		case FREE_INSTRM:
			{
				// const_tree arg0 = gimple_call_arg(stmt, 0);
				gimple call = gimple_build_call (decl4, 1, obj_ptr);
				gimple_stmt_iterator gsi = gsi_for_stmt(stmt);
				gsi_insert_after(&gsi, call, GSI_NEW_STMT);
				break;
			}
	}
	return ;
}

#endif // INSTRUMENT


#ifdef STATICANA
static FILE* fp = NULL;
static char* dump_file_path = (char*)"/home/yueqi/fengshui/corpus/Sa/allnoconfig/free/start/task_struct";

static void open_file(void*, void*) {
	fp = fopen(dump_file_path, "a");
	if (fp == NULL)
		fprintf(stderr, "open kmem_slab_obj_dump err\n");
}

static void close_file(void*, void*) {
	fclose(fp);
}
#endif // STATICANA

static int islegalchain(const char* subfield_type_name) {
	int i = 0;
	for (i = 0; i < subfield_type_array_len; i++) {
		if (strcmp(subfield_type_name, subfield_type_array[i]) == 0)
			return 1;
	}
	return false;
}

// ANALYSIS
static unsigned int instrument_panic_execute(void) 
{
//	if (
//			strcmp(function_name(cfun), "call_usermodehelper_freeinfo") != 0 &&
//			strcmp(function_name(cfun), "call_usermodehelper_exec_async") != 0 &&
//			strcmp(function_name(cfun), "call_usermodehelper_exec") != 0 &&
//		  strcmp(function_name(cfun), "cryptd_hash_enqueue") != 0)
//		return 0;
//	fprintf(stderr, "function name: %s\n", function_name(cfun));

	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun) {
		gimple_stmt_iterator gsi;

#ifdef ALLOC_PLUGIN
		/* 
		* Example: XXX
		*/
		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
			gimple stmt =  gsi_stmt(gsi);
			if (!is_gimple_call(stmt))
				continue;

			const_tree current_fn_decl = gimple_call_fndecl(stmt);
			if (current_fn_decl == NULL_TREE)
				continue;
			// const char* callee_name = function_name(DECL_STRUCT_FUNCTION(current_fn_decl));
			const char* callee_name = IDENTIFIER_POINTER(DECL_NAME(current_fn_decl)); 
			// callee is slab alloc function
			if (!is_slab_alloc_func(callee_name)) 
				continue;

			const_tree lhs = gimple_get_lhs(stmt);
			if (lhs == NULL_TREE)
				continue;

			// return type is pointer
			const_tree lhs_type = TREE_TYPE(lhs);
			if (TREE_CODE(lhs_type) != POINTER_TYPE)
				continue;

			const_tree ptr_lhs_type = TYPE_MAIN_VARIANT(
									strip_array_types(
									TYPE_MAIN_VARIANT(
									TREE_TYPE(lhs_type))));

			const_tree ptr_lhs_type_tree = TYPE_NAME(ptr_lhs_type);

			if (ptr_lhs_type_tree == NULL_TREE)
				continue;
			
			// pointee type name
			const char* type_name = TYPE_NAME_POINTER(ptr_lhs_type);
			if (is_magic_wrong_num(type_name))
				continue;
#ifdef INSTRUMENT
			if (strcmp(type_name, target_object_name) != 0)
				continue;

			fprintf(stderr, "find an allocation site in function %s\n", function_name(cfun));
			instrument_panic(stmt, ALLOC_INSTRM, NULL_TREE);
#endif

#ifdef STATICANA
			fprintf(fp, "%s:%s:%s\n", type_name, callee_name,  function_name(cfun));
#endif
		}
#endif // ALLOC_PLUGIN

#ifdef DEREF_PLUGIN
		/* 
		 * Example: XXX
		 */
		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
			gimple stmt =  gsi_stmt(gsi);
			// ssa = target->subfield
			if (!is_gimple_assign(stmt))
				continue;
			enum tree_code rhs_code = gimple_assign_rhs_code(stmt);
			if (rhs_code != COMPONENT_REF)
				continue;

			// TODO
			// rhs1 can be ahash_request->base.complete
			// in function cryptd_hash_enqueue()
			// which makes following code doesn't work
			// target->subfield
			tree rhs1 = gimple_assign_rhs1(stmt);
			// debug_tree(rhs1);
			// target
			tree rhs1_arg0 = TREE_OPERAND(rhs1, 0);
			const_tree rhs1_arg0_type = TREE_TYPE(rhs1_arg0);

			// TYPE_MAIN_VARIANT is useful when encountering wierd error
			rhs1_arg0_type = TYPE_MAIN_VARIANT(rhs1_arg0_type);

			// record_type represents struct and class types
			if (TREE_CODE(rhs1_arg0_type) != RECORD_TYPE)
				continue;
			if (TYPE_NAME(rhs1_arg0_type) == NULL_TREE)
				continue;
			const char* type_name = TYPE_NAME_POINTER(rhs1_arg0_type);
			if (strcmp(type_name, target_object_name) != 0)
				continue;

			// subfield
			tree rhs1_arg1 = TREE_OPERAND(rhs1, 1);
			enum tree_code rhs1_arg1_code = TREE_CODE(rhs1_arg1);
			assert(rhs1_arg1_code == FIELD_DECL);

			const_tree rhs1_arg1_type = TREE_TYPE(rhs1_arg1);
			// subfield's type can be a pointer or maybe other types XXX
			if (TREE_CODE(rhs1_arg1_type) != POINTER_TYPE)
				continue;

			tree rhs1_arg1_ptr = TYPE_MAIN_VARIANT(strip_array_types(
			                     TYPE_MAIN_VARIANT(TREE_TYPE(rhs1_arg1))
								 ));

			const_tree rhs1_arg1_ptr_type = TREE_TYPE(rhs1_arg1_ptr);
			// subfield as a pointer is pointing at a record type due to multi-deref
			if (TREE_CODE(rhs1_arg1_ptr_type) != RECORD_TYPE)
				continue;
			if (TYPE_NAME(rhs1_arg1_ptr_type) == NULL_TREE)
				continue;
			const char* subfield_type_name = TYPE_NAME_POINTER(rhs1_arg1_ptr_type);
			// fprintf(stderr, "subfield_type_name: %s\n", subfield_type_name);

			if(!islegalchain(subfield_type_name))
				continue;
			
			// This is the name of the object as written by the user.
				//  It is an IDENTIFIER_NODE.  
			const_tree decl_name_identifier = DECL_NAME(rhs1_arg1);
			const char* field_name = IDENTIFIER_POINTER(decl_name_identifier);

			fprintf(stderr, "find a dereferencing site in function %s\n", function_name(cfun));
			fprintf(stderr, "%s->%s\n", type_name, field_name);
			tree ssa_var = gimple_assign_lhs(stmt);
			imm_use_iterator iter;
			gimple imm_use_stmt;
			FOR_EACH_IMM_USE_STMT(imm_use_stmt, iter, ssa_var) {
				// call_func(ssa) OR ssa(para) OR ssa->subfield
				if (is_gimple_call(imm_use_stmt) ||
						is_gimple_assign(imm_use_stmt)) {
					// TODO instrument
					fprintf(stderr, "this one is promising\n");
#ifdef INSTRUMENT
					instrument_panic(stmt, DEREF_INSTRM, NULL_TREE);
#endif

#ifdef STATICANA
					fprintf(fp, "%s\n", function_name(cfun));
#endif
					// debug_gimple_stmt(imm_use_stmt);
				} 
				// XXX may some other situations
			}
		}
#endif // DEREF_PLUGIN

#ifdef RCU_PLUGIN
		/* 
		 * Example: 
		 * _1 = &kioctx_table->rcu
		 * kfree_call_rcu(_1, kfree)
		 */

		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
			gimple stmt = gsi_stmt(gsi);
			if (!is_gimple_call(stmt))
				continue;

			const_tree current_fn_decl = gimple_call_fndecl(stmt);
			if (current_fn_decl == NULL_TREE)
				continue;
			const char* callee_name = IDENTIFIER_POINTER(DECL_NAME(current_fn_decl)); 
			if (!is_rcu_free_func(callee_name))
				continue;

			const_tree arg0 = gimple_call_arg(stmt, 0);
			gimple def_stmt = SSA_NAME_DEF_STMT(arg0);
			if (!is_gimple_assign(def_stmt))
				continue;

			enum tree_code rhs_code = gimple_assign_rhs_code(def_stmt);
			if (rhs_code != ADDR_EXPR)
				continue;

			tree rhs1 = gimple_assign_rhs1(def_stmt); // &kioctx_table->rcu
			tree op0 = TREE_OPERAND(rhs1, 0);

			tree rcu_op0 = TREE_OPERAND(op0, 0); // kioctx_table

			const_tree rcu_op0_type = TREE_TYPE(rcu_op0);
			if (TREE_CODE(rcu_op0_type) ==  RECORD_TYPE) {
				if (TYPE_NAME(rcu_op0_type) == NULL_TREE)
					continue;
				const char* type_name = TYPE_NAME_POINTER(rcu_op0_type);
				if (strcmp(type_name, target_object_name) != 0)
					continue;
				fprintf(stderr, "find %s in function %s\n", callee_name, function_name(cfun));
				tree ssa_op0 = TREE_OPERAND(rcu_op0, 0);
#ifdef INSTRUMENT
				instrument_panic(stmt, RCU_INSTRM, ssa_op0);
#endif

#ifdef STATICANA
				fprintf(fp, "%s\n", function_name(cfun));
#endif
			} else if (TREE_CODE(rcu_op0_type) == UNION_TYPE) {
				tree union_rcu_op0 = TREE_OPERAND(rcu_op0, 0);
				const_tree union_rcu_op0_type =TREE_TYPE(union_rcu_op0);
				if (TREE_CODE(union_rcu_op0_type) != RECORD_TYPE ||
					TYPE_NAME(union_rcu_op0_type) == NULL_TREE)
					continue;
				const char* type_name = TYPE_NAME_POINTER(union_rcu_op0_type);
				if (strcmp(type_name, target_object_name) != 0)
					continue;
				fprintf(stderr, "find %s in function %s\n", callee_name, function_name(cfun));
				tree ssa_op0 = TREE_OPERAND(union_rcu_op0, 0);
#ifdef INSTRUMENT
				instrument_panic(stmt, RCU_INSTRM, ssa_op0);
#endif

#ifdef STATICANA
				fprintf(fp, "%s\n", function_name(cfun));
#endif
			}
		}
#endif // RCU_PLUGIN

#ifdef FREE_PLUGIN
		/* 
		 * Example: 
		 * kfree(table)
		 * kmem_cache_free(epi_cache, epi)
		 */
		 for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
		 	gimple stmt = gsi_stmt(gsi);
            stmt = gsi_stmt(gsi);
            if (!is_gimple_call(stmt))
                continue;

            const_tree current_fn_decl = gimple_call_fndecl(stmt);
            if (current_fn_decl == NULL_TREE)
                continue;
            const char* callee_name = IDENTIFIER_POINTER(DECL_NAME(current_fn_decl));
			tree arg = NULL_TREE;
			if (strcmp(callee_name, "kfree") == 0)
            	arg = gimple_call_arg(stmt, 0);
			else if (strcmp(callee_name, "kmem_cache_free") == 0)
				arg = gimple_call_arg(stmt, 1);
			else
				continue;

            const_tree arg_type = TREE_TYPE(arg);
			if (TREE_CODE(arg_type) != POINTER_TYPE)
				continue;
            const_tree ptr_arg_type = TYPE_MAIN_VARIANT(
							strip_array_types(
							TYPE_MAIN_VARIANT(
							TREE_TYPE(arg_type))));
            const_tree ptr_arg_type_tree = TYPE_NAME(ptr_arg_type);
            if (ptr_arg_type_tree == NULL_TREE)
                continue;
            const char* type_name = TYPE_NAME_POINTER(ptr_arg_type);
			if (is_magic_wrong_num(type_name))
				continue;

            if (strcmp(type_name, target_object_name) != 0)
                continue;	

			fprintf(stderr, "find a freeing site in function %s\n", function_name(cfun));
#ifdef INSTRUMENT
            instrument_panic(stmt, FREE_INSTRM, arg);
#endif

#ifdef STATICANA
			fprintf(fp, "%s\n", function_name(cfun));
#endif
		}
#endif // FREE_PLUGIN

#ifdef TYPE1_PLUGIN
		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
			gimple stmt = gsi_stmt(gsi);
			stmt = gsi_stmt(gsi);
			if (!is_gimple_call(stmt))
				continue;
			const_tree current_fn_decl = gimple_call_fndecl(stmt);
			if (current_fn_decl == NULL_TREE)
				continue;
			const char* callee_name = IDENTIFIER_POINTER(DECL_NAME(current_fn_decl));
			if (strcmp(callee_name, "copy_from_user") != 0 || 
				strcmp(callee_name, "strncpy_from_user") != 0)
				continue;
			fprintf(stderr, "find calling to copy_from_user in function %s\n", function_name(cfun));
#ifdef INSTRUMENT
#endif

#ifdef STATICANA
			fprintf(fp, "%s\n", function_name(cfun));
#endif
		}
#endif // TYPE1_PLUGIN

	}
}

// PLUGIN INIT
#define PASS_NAME instrument_panic
#define NO_GATE
#define TODO_FLAGS_FINISH TODO_dump_func

#include "gcc-generate-gimple-pass.h"

__visible int plugin_init(struct plugin_name_args* plugin_info, struct plugin_gcc_version* version) 
{
	const char* const plugin_name = plugin_info->base_name;

	PASS_INFO(instrument_panic, "ssa", 1, PASS_POS_INSERT_AFTER);

	if (!plugin_default_version_check(version, &gcc_version)) {
		error(G_("incompatible gcc/plugin versions"));
		return 1;
	}

	/* we need a callback at the start of each compilation 
	 * unit to declare the needed funtions. Calling to these
	 * these functions will be instrumented in the code later
	 * during GIMPLE pass
	 */ 
#ifdef INSTRUMENT
	register_callback(plugin_name, PLUGIN_START_UNIT, 
			&decl_panic_func, NULL);
#endif

#ifdef STATICANA
	register_callback(plugin_name, PLUGIN_START_UNIT,
			&open_file, NULL);
#endif

	register_callback(plugin_name, PLUGIN_INFO, NULL, 
			&instrument_panic_plugin_info);
	register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL,
			&instrument_panic_pass_info);

#ifdef STATICANA
	register_callback(plugin_name, PLUGIN_FINISH_UNIT,
			&close_file, NULL);
#endif

	return 0;
}
