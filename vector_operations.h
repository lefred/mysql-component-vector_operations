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

#define LOG_COMPONENT_TAG "community_vector"

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/log_builtins.h> /* LogComponentErr */
#include <mysql/components/services/mysql_runtime_error_service.h>
#include <mysql/components/services/udf_metadata.h>
#include <mysql/components/services/udf_registration.h>
#include <mysqld_error.h> /* Errors */

#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "mysql/strings/m_ctype.h"
#include "sql/field.h"
#include "sql/sql_udf.h"
#include "sql/vector_conversion.h"

void populate_vector(uint32_t vec_dim, float *vec1,
                     std::vector<float> &vector1) {
  vector1.clear();
  vector1.reserve(vec_dim);

  for (uint32_t i = 0; i < vec_dim; i++) {
    float value1;
    memcpy(&value1, vec1 + i, sizeof(float));
    vector1.push_back(value1);
  }
}

std::string std_vector_to_string(const std::vector<float> &vec) {
  std::ostringstream oss;
  oss << "[";

  for (size_t i = 0; i < vec.size(); ++i) {
    // Set precision and scientific notation
    oss << std::scientific << vec[i];

    // Add a comma if it's not the last element
    if (i != vec.size() - 1) {
      oss << ",";
    }
  }

  oss << "]";
  return oss.str();
}

static char *vector_addition(uint32_t vec_dim, float *vec1, float *vec2,
                             unsigned long *length) {
  std::vector<float> vector1;
  std::vector<float> vector2;

  populate_vector(vec_dim, vec1, vector1);
  populate_vector(vec_dim, vec2, vector2);

  std::vector<float> result(vector1.size());
  for (size_t i = 0; i < vector1.size(); ++i) {
    result[i] = vector1[i] + vector2[i];
  }

  std::string result_str = std_vector_to_string(result);
  char *result_cstr = result_str.data();
  String vector_string;

  uint32 output_dims = Field_vector::max_dimensions;
  auto dimension_bytes = Field_vector::dimension_bytes(output_dims);
  if (vector_string.mem_realloc(dimension_bytes)) return 0;

  bool err = from_string_to_vector(result_cstr, strlen(result_cstr),
                                   vector_string.ptr(), &output_dims);

  if (err) {
    if (output_dims == Field_vector::max_dimensions) {
      vector_string.replace(32, 5, "... \0", 5);
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_addition",
                                      "Data out of range");
    } else {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_addition",
                                      "Invalid vector conversion");
    }
    return 0;
  }

  size_t output_vector_length = Field_vector::dimension_bytes(output_dims);
  vector_string.length(output_vector_length);
  *length = output_vector_length;

  return vector_string.c_ptr_safe();
}

static char *vector_subtraction(uint32_t vec_dim, float *vec1, float *vec2,
                                unsigned long *length) {
  std::vector<float> vector1;
  std::vector<float> vector2;

  populate_vector(vec_dim, vec1, vector1);
  populate_vector(vec_dim, vec2, vector2);

  std::vector<float> result(vector1.size());
  for (size_t i = 0; i < vector1.size(); ++i) {
    result[i] = vector1[i] - vector2[i];
  }

  std::string result_str = std_vector_to_string(result);
  char *result_cstr = result_str.data();
  String vector_string;

  uint32 output_dims = Field_vector::max_dimensions;
  auto dimension_bytes = Field_vector::dimension_bytes(output_dims);
  if (vector_string.mem_realloc(dimension_bytes)) return 0;

  bool err = from_string_to_vector(result_cstr, strlen(result_cstr),
                                   vector_string.ptr(), &output_dims);

  if (err) {
    if (output_dims == Field_vector::max_dimensions) {
      vector_string.replace(32, 5, "... \0", 5);
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_subtraction",
                                      "Data out of range");
    } else {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_subtraction",
                                      "Invalid vector conversion");
    }
    return 0;
  }

  size_t output_vector_length = Field_vector::dimension_bytes(output_dims);
  vector_string.length(output_vector_length);
  *length = output_vector_length;

  return vector_string.c_ptr_safe();
}

static char *vector_multiplication(uint32_t vec_dim, float *vec1, float *vec2,
                                   unsigned long *length) {
  std::vector<float> vector1;
  std::vector<float> vector2;

  populate_vector(vec_dim, vec1, vector1);
  populate_vector(vec_dim, vec2, vector2);

  std::vector<float> result(vector1.size());
  for (size_t i = 0; i < vector1.size(); ++i) {
    result[i] = vector1[i] * vector2[i];
  }

  std::string result_str = std_vector_to_string(result);
  char *result_cstr = result_str.data();
  String vector_string;

  uint32 output_dims = Field_vector::max_dimensions;
  auto dimension_bytes = Field_vector::dimension_bytes(output_dims);
  if (vector_string.mem_realloc(dimension_bytes)) return 0;

  bool err = from_string_to_vector(result_cstr, strlen(result_cstr),
                                   vector_string.ptr(), &output_dims);

  if (err) {
    if (output_dims == Field_vector::max_dimensions) {
      vector_string.replace(32, 5, "... \0", 5);
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_multiplication",
                                      "Data out of range");
    } else {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_multiplication",
                                      "Invalid vector conversion");
    }
    return 0;
  }

  size_t output_vector_length = Field_vector::dimension_bytes(output_dims);
  vector_string.length(output_vector_length);
  *length = output_vector_length;

  return vector_string.c_ptr_safe();
}

static char *vector_division(uint32_t vec_dim, float *vec1, float *vec2,
                             unsigned long *length) {
  std::vector<float> vector1;
  std::vector<float> vector2;

  populate_vector(vec_dim, vec1, vector1);
  populate_vector(vec_dim, vec2, vector2);

  std::vector<float> result(vector1.size());
  for (size_t i = 0; i < vector1.size(); ++i) {
    if (vector2[i] == 0) {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_division",
                                      "Division by zero is undefined");
      return 0;
    }
    result[i] = vector1[i] / vector2[i];
  }

  std::string result_str = std_vector_to_string(result);
  char *result_cstr = result_str.data();
  String vector_string;

  uint32 output_dims = Field_vector::max_dimensions;
  auto dimension_bytes = Field_vector::dimension_bytes(output_dims);
  if (vector_string.mem_realloc(dimension_bytes)) return 0;

  bool err = from_string_to_vector(result_cstr, strlen(result_cstr),
                                   vector_string.ptr(), &output_dims);

  if (err) {
    if (output_dims == Field_vector::max_dimensions) {
      vector_string.replace(32, 5, "... \0", 5);
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_division",
                                      "Data out of range");
    } else {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                                      ER_UDF_ERROR, 0, "vector_division",
                                      "Invalid vector conversion");
    }
    return 0;
  }

  size_t output_vector_length = Field_vector::dimension_bytes(output_dims);
  vector_string.length(output_vector_length);
  *length = output_vector_length;

  return vector_string.c_ptr_safe();
}

extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);
extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

extern SERVICE_TYPE(log_builtins) * log_bi;
extern SERVICE_TYPE(log_builtins_string) * log_bs;
