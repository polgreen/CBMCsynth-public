//
// Created by julian on 24.04.23.
//

#ifndef CBMCSMT_UTIL_H
#define CBMCSMT_UTIL_H

#include <utility>
#include <filesystem>

#include <util/cmdline.h>
#include <util/cout_message.h>
#include <util/format_expr.h>
#include <util/namespace.h>
#include <util/replace_symbol.h>
#include <util/simplify_expr.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>
#include <util/expr.h>
#include <util/mathematical_expr.h>

#include "../constants.h"

bool root_equality(const exprt &a, const exprt &b);

function_application_exprt
create_func_app(irep_idt function_name, const std::vector<exprt> &operands, const typet &codomain);

void expand_let_expressions(exprt &expr);

void print_subterms_and_types(const exprt &expr);

template<typename T>
void concat(std::vector<T> &A, const std::vector<T> &B) {
    A.insert(A.end(), B.begin(), B.end());
}

std::size_t expr_height(const exprt &expr);

std::vector<std::string> files_with_suffix_in_dirs(const std::vector<std::string> &dir, const std::string &suffix);

std::size_t is_binder_free(const exprt &expr);

/*
 * See if what is a subterm of in
 * */
bool is_subterm(const exprt &what, const exprt &in);

std::string replace_occurences(std::string str, const std::string &from, const std::string &to);

bool create_dir_recursively(const std::filesystem::path& dirName);

void nnf(exprt &expr, bool negate);
void dnf(exprt &expr);

#endif //CBMCSMT_UTIL_H
