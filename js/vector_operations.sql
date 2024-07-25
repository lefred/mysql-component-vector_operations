drop function if exists vector_addition_js;
create function vector_addition_js(a varchar(15000), b varchar(15000))
    returns varchar(15000) language javascript as $$
    const vec1 = JSON.parse(a)
    const vec2 = JSON.parse(b)
    if (vec1.length !== vec2.length) {
        throw new Error("Vectors must have the same dimension")
    }
    const result = []
    let i = 0
    while (i < vec1.length) {
        result.push(vec1[i] + vec2[i])
        i++
    }
    const resultStr = JSON.stringify(result)
    try {
        const parsedResult = JSON.parse(resultStr)
        return resultStr
    } catch (error) {
        throw new Error("Invalid vector conversion")
    }

$$;

drop function if exists vector_subtraction_js;
create function vector_subtraction_js(a varchar(15000), b varchar(15000))
    returns varchar(15000) language javascript as $$
    const vec1 = JSON.parse(a)
    const vec2 = JSON.parse(b)
    if (vec1.length !== vec2.length) {
        throw new Error("Vectors must have the same dimension")
    }
    const result = []
    let i = 0
    while (i < vec1.length) {
        result.push(vec1[i] - vec2[i])
        i++
    }
    const resultStr = JSON.stringify(result)
    try {
        const parsedResult = JSON.parse(resultStr)
        return resultStr
    } catch (error) {
        throw new Error("Invalid vector conversion")
    }

$$;

drop function if exists vector_multiplication_js;
create function vector_multiplication_js(a varchar(15000), b varchar(15000))
    returns varchar(15000) language javascript as $$
    const vec1 = JSON.parse(a)
    const vec2 = JSON.parse(b)
    if (vec1.length !== vec2.length) {
        throw new Error("Vectors must have the same dimension")
    }
    const result = []
    let i = 0
    while (i < vec1.length) {
        result.push(vec1[i] * vec2[i])
        i++
    }
    const resultStr = JSON.stringify(result)
    try {
        const parsedResult = JSON.parse(resultStr)
        return resultStr
    } catch (error) {
        throw new Error("Invalid vector conversion")
    }

$$;

drop function if exists vector_division_js;
create function vector_division_js(a varchar(15000), b varchar(15000))
    returns varchar(15000) language javascript as $$
    const vec1 = JSON.parse(a)
    const vec2 = JSON.parse(b)
    if (vec1.length !== vec2.length) {
        throw new Error("Vectors must have the same dimension")
    }
    const result = []
    let i = 0
    while (i < vec1.length) {
        if (vec2[i] == 0) {
           throw new Error("Division by zero is undefined")
        }
        result.push(vec1[i] / vec2[i])
        i++
    }
    const resultStr = JSON.stringify(result)
    try {
        const parsedResult = JSON.parse(resultStr)
        return resultStr
    } catch (error) {
        throw new Error("Invalid vector conversion")
    }

$$;
