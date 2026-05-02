#include <mysql/mysql.h>
#include <mysql/udf_registration_types.h>
#include <string.h>
#include <unordered_map>
#include <cstdint>

struct ModeData {
    std::unordered_map<double, uint64_t> counts;
};

extern "C" {
    bool mode_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
        if(args->arg_count != 1) {
            strcpy(message, "mode() requires one argument");
            return true;
        }

        ModeData *data = new ModeData;

        args->arg_type[0] = REAL_RESULT;

        initid->ptr = (char *)data;
        initid->maybe_null = 1;
        return false;
    }
    void mode_clear(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        ModeData *data = (ModeData *)initid->ptr;
        data->counts.clear();
    }
    void mode_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        if(args->args[0] == NULL) {return;}
        ModeData *data = (ModeData *)initid->ptr;
        data->counts[*((double*)args->args[0])]++;
    }
    double mode(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
        ModeData *data = (ModeData *)initid->ptr;
        if(data->counts.empty()) {
            *is_null = 1;
            return 0.0;
        }
        double best_value = 0.0;
        uint64_t best_count = 0;
        for(const auto &kv : data->counts) {
            if(kv.second > best_count) {
                best_count = kv.second;
                best_value = kv.first;
            }
        }
        return best_value;
    }
    void mode_deinit(UDF_INIT *initid) {
        delete (ModeData *)initid->ptr;
    }
}
