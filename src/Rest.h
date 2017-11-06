
class Rest {

    public:
        //These return the http error code
        static int Post(const std::string &URL, const Json::Value &Request, Json::Value &Response);

};
