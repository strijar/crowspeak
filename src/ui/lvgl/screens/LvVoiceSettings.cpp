#include "LvSettingsScreen.h"
void LvSettingsScreen::buildVoiceItems(int& idx) {
    auto& settings=_cfg->settings().voice;const int start=idx;
    _items.push_back({"Playback volume",SettingType::INTEGER,[&settings] {return int(settings.volume);},[&settings](int value) {settings.volume=value;},[](int value) {return String(value)+"%";},0,100,10});_items.back().id = SettingId::PlaybackVolume;++idx;
    _items.push_back({"Max voice messages",SettingType::INTEGER,[&settings] {return int(settings.maxMessages);},[&settings](int value) {settings.maxMessages=value;},nullptr,1,50,1});_items.back().id = SettingId::MaxVoiceMessages;++idx;
    _categories.push_back({"Voice Messages",start,idx-start,[&settings] {
        return String(settings.volume)+"% volume";
    }});
    _categories.back().id = SettingsCategoryId::Voice;
}
