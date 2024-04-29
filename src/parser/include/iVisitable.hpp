#include "iVisitor.hpp"

class IVisitable
{
public:
    virtual void accept(IVisitor &visitor) = 0;
    virtual ~IVisitable() = default;
};
