#include "engine_panel.hpp"
#include "tableaus.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

namespace phys_env
{
    engine_panel::engine_panel(rk::integrator &integ, float &dt) : m_integ(integ), m_dt(dt) {}

    void engine_panel::render(const float integ_time, int &integ_per_frame)
    {
        ImGui::Begin("Engine");
        ImGui::SetWindowFontScale(3.f);
        ImGui::Text("Elapsed integration time: %.2f", integ_time);
        if (m_integ.tableau().embedded())
            ImGui::Text("Integration error: %e", m_integ.error());
        render_sliders(integ_per_frame);
        render_methods_list();
        update_method_if_changed();
        ImGui::End();
    }

    void engine_panel::render_sliders(int &integ_per_frame) const
    {
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("Integration timestep", &m_dt, 1.e-5f, 1.e-1f, "%.5f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderInt("Integrations per frame", &integ_per_frame, 1, 100);
        ImGui::PopItemWidth();
    }

    void engine_panel::render_methods_list() const
    {
        ImGui::PushItemWidth(200);
        const char *const methods[] = {"RK1",
                                       "RK2",
                                       "RK4",
                                       "RK38",
                                       "RKF12",
                                       "RKF45",
                                       "RKFCK45",
                                       "RKF78"};
        ImGui::ListBox("Integration method", (int *)&m_method, methods, IM_ARRAYSIZE(methods));
        ImGui::PopItemWidth();
    }

    void engine_panel::update_method_if_changed()
    {
        if (m_method == m_last_method)
            return;
        switch (m_method)
        {
        case RK1:
            m_integ.tableau(rk::rk1);
            break;
        case RK2:
            m_integ.tableau(rk::rk2);
            break;
        case RK4:
            m_integ.tableau(rk::rk4);
            break;
        case RK38:
            m_integ.tableau(rk::rk38);
            break;
        case RKF12:
            m_integ.tableau(rk::rkf12);
            break;
        case RKF45:
            m_integ.tableau(rk::rkf45);
            break;
        case RKFCK45:
            m_integ.tableau(rk::rkfck45);
            break;
        case RKF78:
            m_integ.tableau(rk::rkf78);
            break;
        }
        m_last_method = m_method;
    }
}