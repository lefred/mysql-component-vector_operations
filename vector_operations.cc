/* Copyright (c) 2017, 2024, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#define NO_SIGNATURE_CHANGE 0
#define SIGNATURE_CHANGE 1

#include "vector_operations.h"

REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);
REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

SERVICE_TYPE(log_builtins) * log_bi;
SERVICE_TYPE(log_builtins_string) * log_bs;

class udf_list {
  typedef std::list<std::string> udf_list_t;

 public:
  ~udf_list() { unregister(); }
  bool add_scalar(const char *func_name, enum Item_result return_type,
                  Udf_func_any func, Udf_func_init init_func = NULL,
                  Udf_func_deinit deinit_func = NULL) {
    if (!mysql_service_udf_registration->udf_register(
            func_name, return_type, func, init_func, deinit_func)) {
      set.push_back(func_name);
      return false;
    }
    return true;
  }

  bool unregister() {
    udf_list_t delete_set;
    /* try to unregister all of the udfs */
    for (auto udf : set) {
      int was_present = 0;
      if (!mysql_service_udf_registration->udf_unregister(udf.c_str(),
                                                          &was_present) ||
          !was_present)
        delete_set.push_back(udf);
    }

    /* remove the unregistered ones from the list */
    for (auto udf : delete_set) set.remove(udf);

    /* success: empty set */
    if (set.empty()) return false;

    /* failure: entries still in the set */
    return true;
  }

 private:
  udf_list_t set;
} *list;

namespace udf_impl {

void error_msg_size() {
  mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                  ER_UDF_ERROR, 0, "vector operation",
                                  "both vectors must have the same size");
}

const char *udf_init = "udf_init", *my_udf = "my_udf",
           *my_udf_clear = "my_clear", *my_udf_add = "my_udf_add";

// UDF to implement a vector addition function between two vectors

static bool vector_addition_udf_init(UDF_INIT *initid, UDF_ARGS *args, char *) {
  if (args->arg_count < 2) {
    mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                    ER_UDF_ERROR, 0, "vector_addition",
                                    "this function requires 2 parameters");
    return true;
  }
  initid->maybe_null = true;
  return false;
}

static void vector_addition_udf_deinit(__attribute__((unused))
                                       UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

const char *vector_addition_udf(UDF_INIT *, UDF_ARGS *args, char *vector_sum,
                                unsigned long *length, char *is_null,
                                char *error) {
  *error = 0;
  *is_null = 0;

  uint32_t dim_vec1 = get_dimensions(args->lengths[0], sizeof(float));
  uint32_t dim_vec2 = get_dimensions(args->lengths[1], sizeof(float));
  if (dim_vec1 != dim_vec2 || dim_vec1 == UINT32_MAX ||
      dim_vec2 == UINT32_MAX) {
    error_msg_size();
    *error = 1;
    *is_null = 1;
    return 0;
  }

  float *vec1 = ((float *)args->args[0]);
  float *vec2 = ((float *)args->args[1]);

  unsigned long vector_length = 0;

  vector_sum = vector_addition(dim_vec1, vec1, vec2, &vector_length);
  *length = vector_length;

  return const_cast<char *>(vector_sum);
}

// UDF to implement a vector subtraction function between two vectors

static bool vector_subtraction_udf_init(UDF_INIT *initid, UDF_ARGS *args,
                                        char *) {
  if (args->arg_count < 2) {
    mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                    ER_UDF_ERROR, 0, "vector_subtraction",
                                    "this function requires 2 parameters");
    return true;
  }
  initid->maybe_null = true;
  return false;
}

static void vector_subtraction_udf_deinit(__attribute__((unused))
                                          UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

const char *vector_subtraction_udf(UDF_INIT *, UDF_ARGS *args, char *vector_sum,
                                   unsigned long *length, char *is_null,
                                   char *error) {
  *error = 0;
  *is_null = 0;

  uint32_t dim_vec1 = get_dimensions(args->lengths[0], sizeof(float));
  uint32_t dim_vec2 = get_dimensions(args->lengths[1], sizeof(float));
  if (dim_vec1 != dim_vec2 || dim_vec1 == UINT32_MAX ||
      dim_vec2 == UINT32_MAX) {
    error_msg_size();
    *error = 1;
    *is_null = 1;
    return 0;
  }

  float *vec1 = ((float *)args->args[0]);
  float *vec2 = ((float *)args->args[1]);

  unsigned long vector_length = 0;

  vector_sum = vector_subtraction(dim_vec1, vec1, vec2, &vector_length);
  *length = vector_length;

  return const_cast<char *>(vector_sum);
}

// UDF to implement a vector product function of two vectors

static bool vector_multiplication_udf_init(UDF_INIT *initid, UDF_ARGS *args,
                                           char *) {
  if (args->arg_count < 2) {
    mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                    ER_UDF_ERROR, 0, "vector_multiplication",
                                    "this function requires 2 parameters");
    return true;
  }
  initid->maybe_null = true;
  return false;
}

static void vector_multiplication_udf_deinit(__attribute__((unused))
                                             UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

const char *vector_multiplication_udf(UDF_INIT *, UDF_ARGS *args,
                                      char *vector_sum, unsigned long *length,
                                      char *is_null, char *error) {
  *error = 0;
  *is_null = 0;

  uint32_t dim_vec1 = get_dimensions(args->lengths[0], sizeof(float));
  uint32_t dim_vec2 = get_dimensions(args->lengths[1], sizeof(float));
  if (dim_vec1 != dim_vec2 || dim_vec1 == UINT32_MAX ||
      dim_vec2 == UINT32_MAX) {
    error_msg_size();
    *error = 1;
    *is_null = 1;
    return 0;
  }

  float *vec1 = ((float *)args->args[0]);
  float *vec2 = ((float *)args->args[1]);

  unsigned long vector_length = 0;

  vector_sum = vector_multiplication(dim_vec1, vec1, vec2, &vector_length);
  *length = vector_length;

  return const_cast<char *>(vector_sum);
}

// UDF to implement a vector division function of two vectors

static bool vector_division_udf_init(UDF_INIT *initid, UDF_ARGS *args, char *) {
  if (args->arg_count < 2) {
    mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                    ER_UDF_ERROR, 0, "vector_division",
                                    "this function requires 2 parameters");
    return true;
  }
  initid->maybe_null = true;
  return false;
}

static void vector_division_udf_deinit(__attribute__((unused))
                                       UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

const char *vector_division_udf(UDF_INIT *, UDF_ARGS *args, char *vector_sum,
                                unsigned long *length, char *is_null,
                                char *error) {
  *error = 0;
  *is_null = 0;

  uint32_t dim_vec1 = get_dimensions(args->lengths[0], sizeof(float));
  uint32_t dim_vec2 = get_dimensions(args->lengths[1], sizeof(float));
  if (dim_vec1 != dim_vec2 || dim_vec1 == UINT32_MAX ||
      dim_vec2 == UINT32_MAX) {
    error_msg_size();
    *error = 1;
    *is_null = 1;
    return 0;
  }

  float *vec1 = ((float *)args->args[0]);
  float *vec2 = ((float *)args->args[1]);

  unsigned long vector_length = 0;

  vector_sum = vector_division(dim_vec1, vec1, vec2, &vector_length);
  *length = vector_length;

  return const_cast<char *>(vector_sum);
}

} /* namespace udf_impl */

static mysql_service_status_t vector_operations_service_init() {
  mysql_service_status_t result = 0;

  log_bi = mysql_service_log_builtins;
  log_bs = mysql_service_log_builtins_string;

  LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, "initializing…");

  list = new udf_list();

  if (list->add_scalar("VECTOR_ADDITION", Item_result::STRING_RESULT,
                       (Udf_func_any)udf_impl::vector_addition_udf,
                       udf_impl::vector_addition_udf_init,
                       udf_impl::vector_addition_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  if (list->add_scalar("VECTOR_SUBTRACTION", Item_result::STRING_RESULT,
                       (Udf_func_any)udf_impl::vector_subtraction_udf,
                       udf_impl::vector_subtraction_udf_init,
                       udf_impl::vector_subtraction_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  if (list->add_scalar("VECTOR_MULTIPLICATION", Item_result::STRING_RESULT,
                       (Udf_func_any)udf_impl::vector_multiplication_udf,
                       udf_impl::vector_multiplication_udf_init,
                       udf_impl::vector_multiplication_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  if (list->add_scalar("VECTOR_DIVISION", Item_result::STRING_RESULT,
                       (Udf_func_any)udf_impl::vector_division_udf,
                       udf_impl::vector_division_udf_init,
                       udf_impl::vector_division_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  return result;
}

static mysql_service_status_t vector_operations_service_deinit() {
  mysql_service_status_t result = 0;

  if (list->unregister()) return 1; /* failure: some UDFs still in use */

  delete list;

  LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, "uninstalled.");

  return result;
}

BEGIN_COMPONENT_PROVIDES(vector_operations_service)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(vector_operations_service)
REQUIRES_SERVICE(log_builtins), REQUIRES_SERVICE(log_builtins_string),
    REQUIRES_SERVICE(mysql_udf_metadata), REQUIRES_SERVICE(udf_registration),
    REQUIRES_SERVICE(mysql_runtime_error), END_COMPONENT_REQUIRES();

/* A list of metadata to describe the Component. */
BEGIN_COMPONENT_METADATA(vector_operations_service)
METADATA("mysql.author", "Oracle Corporation / lefred"),
    METADATA("mysql.license", "GPL"), METADATA("mysql.dev", "lefred"),
    END_COMPONENT_METADATA();

/* Declaration of the Component. */
DECLARE_COMPONENT(vector_operations_service, "mysql:vector_operations_service")
vector_operations_service_init,
    vector_operations_service_deinit END_DECLARE_COMPONENT();

/* Defines list of Components contained in this library. Note that for now
  we assume that library will have exactly one Component. */
DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(vector_operations_service)
    END_DECLARE_LIBRARY_COMPONENTS
