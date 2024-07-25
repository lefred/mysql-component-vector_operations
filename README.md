# mysql-component-vector_operations

Extending MySQL using the Component Infrastructure - Adding 4 basic mathematical operations 
for vectors of the same size.

This code  is not intended for production use and is provided solely for illustrative purposes of a blog
post series related to the VECTOR datatype in MySQL.

The VECTOR datatype was introduced in MySQL 9.0.

```
 MySQL > install component "file://component_vector_operations";
 Query OK, 0 rows affected (0.0005 sec)
 
 MySQL > SELECT UDF_NAME FROM performance_schema.user_defined_functions  WHERE UDF_NAME LIKE 'VECTOR%';
+-----------------------+
| UDF_NAME              |
+-----------------------+
| VECTOR_DIVISION       |
| VECTOR_MULTIPLICATION |
| VECTOR_SUBTRACTION    |
| VECTOR_ADDITION       |
+-----------------------+
4 rows in set (0.0016 sec)

MySQL > SELECT 
          VECTOR_ADDITION(
             STRING_TO_VECTOR('[1,2,3]'), 
             STRING_TO_VECTOR('[4,5,6]')
          ) result;
+----------------------------+
| result                     |
+----------------------------+
| 0x0000A0400000E04000001041 |
+----------------------------+
1 row in set (0.0006 sec)

MySQL > SELECT VECTOR_TO_STRING(
          VECTOR_ADDITION(
             STRING_TO_VECTOR('[1,2,3]'),
             STRING_TO_VECTOR('[4,5,6]')
          )
        ) result;
+---------------------------------------+
| result                                |
+---------------------------------------+
| [5.00000e+00,7.00000e+00,9.00000e+00] |
+---------------------------------------+
1 row in set (0.0002 sec)


MySQL > SELECT VECTOR_TO_STRING(
          VECTOR_DIVISION(
             STRING_TO_VECTOR('[1,2,3]'), 
             STRING_TO_VECTOR('[4,5,6]')
          )
        ) result;
+---------------------------------------+
| result                                |
+---------------------------------------+
| [2.50000e-01,4.00000e-01,5.00000e-01] |
+---------------------------------------+
1 row in set (0.0003 sec)
```

## Errors Handling

```
MySQL > SELECT VECTOR_TO_STRING(
           VECTOR_ADDITION(
              STRING_TO_VECTOR('[]'),
              STRING_TO_VECTOR('[]')
           )
          ) result;
ERROR: 6138 (HY000): Data cannot be converted to a valid vector: '[]'

MySQL > SELECT VECTOR_TO_STRING(
          VECTOR_DIVISION(
             STRING_TO_VECTOR('[1,2,3]'),
             STRING_TO_VECTOR('[4,5,6,2]')
          )
        ) result;
vector operation UDF failed; both vectors must have the same size

MySQL > SELECT VECTOR_TO_STRING(
          VECTOR_DIVISION(
             STRING_TO_VECTOR('[1,2,3]'),
             STRING_TO_VECTOR('[4,5,0]')
          )
        ) result;
vector_division UDF failed; Division by zero is undefined
```

