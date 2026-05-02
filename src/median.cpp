#include <mysql/mysql.h>
#include <mysql/udf_registration_types.h>
#include <string.h>
#include <vector>
#include <algorithm>

struct MedianData {
    std::vector<double> values;
};

extern "C" {
    bool median_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
        if(args->arg_count != 1) {
            strcpy(message, "median() requires one argument");
            return true;
        }

        MedianData *data = new MedianData;

        args->arg_type[0] = REAL_RESULT;

        initid->ptr = (char *)data;
        initid->maybe_null = 1;
        return false;
    }
    void median_clear(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        MedianData *data = (MedianData *)initid->ptr;
        data->values.clear();
    }
    void median_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        if(args->args[0] == NULL) {return;}
        MedianData *data = (MedianData *)initid->ptr;
        data->values.push_back(*((double*)args->args[0]));
    }
    double median(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        MedianData *data = (MedianData *)initid->ptr;
        if(data->values.empty()) {
            *is_null = 1;
            return 0.0;
        }
        size_t n = data->values.size();
        size_t mid = n / 2;
        std::nth_element(data->values.begin(), data->values.begin() + mid, data->values.end());
        double upper = data->values[mid];
        if(n % 2 == 1) {
            return upper;
        }
        double lower = *std::max_element(data->values.begin(), data->values.begin() + mid);
        return (lower + upper) / 2.0;
    }
    void median_deinit(UDF_INIT *initid) {
        delete (MedianData *)initid->ptr;
    }
}
