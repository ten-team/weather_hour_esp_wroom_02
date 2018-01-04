
class YudetamagoClient
{
public:
    static bool GetExistance(const char *objectId, bool& exists);
    static bool SetExistance(const char *objectId, bool exists);
};
