#include "ppx-demo/internal/pch.hpp"
#include "ppx-demo/app/demo_layer.hpp"
#include "ppx-demo/app/demo_app.hpp"

namespace ppx::demo
{
void demo_layer::on_attach()
{
    m_app = parent<demo_app>();
}
} // namespace ppx::demo