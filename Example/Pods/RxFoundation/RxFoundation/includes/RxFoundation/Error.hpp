//
//  Error.hpp
//  RxFoundation
//
//  Created by closure on 10/29/16.
//  Copyright © 2016 closure. All rights reserved.
//

#ifndef Error_hpp
#define Error_hpp

#include <RxFoundation/SharedPointer.hpp>
#include <RxFoundation/String.hpp>

namespace Rx {
    class Error : public SharedFromThis<Error> {
    public:
        Error(Integer code = 0, String message = "") : _code(code), _message(message) {}
        Error(const Error &value) : _code(value._code), _message(value._message) {}
        Error(Error &&value) : _code(std::move(value._code)), _message(std::move(value._message)) { }
        virtual ~Error() {};
    public:
        Integer getCode() const { return _code; }
        Integer getCode() { return _code; }
        
        const String &getMessage() const { return _message; }
        String getMessage() { return _message; }
    private:
        const Integer _code;
        const String _message;
    };
    
    typedef SharedPtr<Error> ErrorPtr;
    typedef SharedRef<Error> ErrorRef;
}

#endif /* Error_hpp */
