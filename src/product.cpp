#include <mysql/mysql.h>
#include <mysql/udf_registration_types.h>
#include <string.h>

struct ProductData {
    double result;
};

extern "C" {
    bool product_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
        if(args->arg_count != 1) {
            strcpy(message, "product() requires one argument");
            return true;
        }
        
        ProductData *data = new ProductData;
        data->result = 1.0;

        args->arg_type[0] = REAL_RESULT;
        
        initid->ptr = (char *)data;
        return false;
    }
    void product_clear(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        ProductData *data = (ProductData *)initid->ptr;
        data->result = 1.0;
    }
    void product_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        if(args->args[0] == NULL) {return;}
        ProductData *data = (ProductData *)initid->ptr;
        data->result *= *((double*)args->args[0]);
    }     
    double product(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        ProductData *data = (ProductData *)initid->ptr;
        return data->result;
    }
    void product_deinit(UDF_INIT *initid) {
        delete (ProductData *)initid->ptr;
    }
}