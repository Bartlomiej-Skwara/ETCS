#include "mode_profile.h"
#include "../Procedures/mode_transition.h"
std::list<mode_profile> mode_profiles;
bool in_mode_ack_area;
bool mode_timer_started = false;
int64_t mode_timer;
optional<mode_profile> requested_mode_profile;
void update_mode_profile()
{
    if (mode_timer_started && mode_timer + T_ACK*1000 < get_milliseconds()) {
        mode_timer_started = false;
        brake_conditions.push_back({-1, nullptr, [](brake_command_information &i) {
            if (mode_acknowledged || mode_to_ack != mode || !mode_acknowledgeable)
                return true;
            return false;
        }});
    }
    in_mode_ack_area = false;
    if (mode_profiles.empty()) {
        requested_mode_profile = {};
        return;
    }
    mode_profile first = mode_profiles.front();
    if (first.mode != Mode::SH && first.length < std::numeric_limits<float>::max() && first.start + first.length < d_minsafefront(first.start)) {
        mode_profiles.pop_front();
        update_mode_profile();
        calculate_SvL();
        return;
    }
    if (mode_profiles.size() > 1) {
        mode_profile second = *(++mode_profiles.begin());
        if (second.start < d_maxsafefront(second.start)) {
            mode_profiles.pop_front();
            update_mode_profile();
            calculate_SvL();
            return;
        }
    }
    for (auto it = mode_profiles.begin(); it != mode_profiles.end(); ++it) {
        mode_profile &p = *it;
        if (d_estfront > p.start-p.acklength) {
            if (mode_acknowledged && mode_to_ack == p.mode) {
                if (it != mode_profiles.begin()) {
                    mode_profiles.erase(mode_profiles.begin(), it);
                    calculate_SvL();
                }
                return;
            }
            in_mode_ack_area = true;
            requested_mode_profile = p;
            if (mode != p.mode && V_est < p.speed) {
                mode_acknowledgeable = true;
                mode_acknowledged = false;
                mode_to_ack = first.mode;
                break;
            }
        }
    }
    if (d_maxsafefront(first.start) > first.start) {
        requested_mode_profile = first;
        if (mode != first.mode) {
            mode_timer_started = true;
            mode_timer = get_milliseconds();
            mode_acknowledgeable = true;
            mode_acknowledged = false;
            mode_to_ack = first.mode;
        }
    }
}
void reset_mode_profile(distance ref, bool infill)
{
    if (infill) {
        for (auto it = mode_profiles.begin(); it != mode_profiles.end(); ++it) {
            if (it->start >= ref) {
                mode_profiles.erase(it, mode_profiles.end());
                break;
            }
        }
    } else {
        mode_profiles.clear();
    }
    if ((mode_to_ack == Mode::OS || mode_to_ack == Mode::LS || mode_to_ack == Mode::SH) && mode_to_ack != mode) {
        mode_acknowledgeable = mode_acknowledged = false;
    }
    calculate_SvL();
}
void set_mode_profile(ModeProfile profile, distance ref, bool infill)
{
    reset_mode_profile(ref, infill);
    std::vector<MP_element_packet> mps;
    mps.push_back(profile.element);
    mps.insert(mps.end(), profile.elements.begin(), profile.elements.end());
    distance start = ref;
    for (auto it = mps.begin(); it != mps.end(); ++it) {
        start += it->D_MAMODE.get_value(profile.Q_SCALE);
        mode_profile p;
        p.start = start;
        p.length = it->L_MAMODE == L_MAMODE_t::Infinity ? std::numeric_limits<float>::max() : it->L_MAMODE.get_value(profile.Q_SCALE);
        p.acklength = it->L_ACKMAMODE.get_value(profile.Q_SCALE);
        switch (it->M_MAMODE)
        {
            case M_MAMODE_t::OS:
                p.mode = Mode::OS;
                p.speed = V_NVONSIGHT;
                break;
            case M_MAMODE_t::LS:
                p.mode = Mode::LS;
                p.speed = V_NVLIMSUPERV;
                break;
            case M_MAMODE_t::SH:
                p.mode = Mode::SH;
                p.speed = V_NVSHUNT;
                break;
        }
        p.start_SvL = it->Q_MAMODE==Q_MAMODE_t::BeginningIsSvL;
        if (it->V_MAMODE != V_MAMODE_t::UseNationalValue)
            p.speed = it->V_MAMODE.get_value();
        mode_profiles.push_back(p);
    }
    calculate_SvL();
}