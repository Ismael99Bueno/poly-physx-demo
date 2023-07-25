#ifndef PPX_DEMO_APP_HPP
#define PPX_DEMO_APP_HPP

#include "ppx-app/app.hpp"

namespace ppx::demo
{
class demo_app : public app
{
  public:
    static demo_app &get();

  private:
    demo_app();
};
} // namespace ppx::demo

#endif