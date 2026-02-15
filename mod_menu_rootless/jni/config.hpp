#ifndef MOD_CONFIG_HPP
#define MOD_CONFIG_HPP

struct ModConfig {
    bool esp_enabled = false;
    bool esp_skeleton = false;
    bool esp_box = false;
    bool esp_distance = false;
    bool esp_health = false;
    bool esp_name = false;
    
    bool aimbot_enabled = false;
    bool aimbot_visible_only = true;
    bool aimbot_team_check = true;
    float aimbot_fov = 90.0f;
    float aimbot_smooth = 5.0f;
    
    bool menu_visible = true;
};

#endif // MOD_CONFIG_HPP
