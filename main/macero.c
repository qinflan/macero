#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "driver/gpio.h"

// screen
#define tag "SSD1306"

// buttons
#define BUTTON_PIN1 25
#define BUTTON_PIN2 26
#define BUTTON_PIN3 27
#define LED_PIN 2 // led

#define DEBOUNCE_TIME 200 / portTICK_PERIOD_MS

// struct for use of menu navigation
typedef struct MenuNode {
	char* options[10];
	int num_options;
	struct MenuNode** children;
	int num_children;
	struct MenuNode* parent;
	void (*actions[10])(void); // limit of 4 functions per menu
} MenuNode;


MenuNode mainMenu;
MenuNode wifiSettings;
MenuNode evilPortalSettings;
MenuNode ddosSettings;
MenuNode snifferSettings;
MenuNode bluetoothSettings;

// TODO refactor active mode functions to other files with their own CMake
void start_evil_portal() {
    gpio_set_level(LED_PIN, 1);
    ESP_LOGI(tag, "Starting Evil AP mode...");
	ESP_LOGI(tag, "LED current level: %d", gpio_get_level(LED_PIN));  
}

void start_ble_spam_attack() {
    gpio_set_level(LED_PIN, 1); 
    ESP_LOGI(tag, "Starting spam attack...");
	ESP_LOGI(tag, "LED current level: %d", gpio_get_level(LED_PIN));  
}

void start_ddos_attack() {
    gpio_set_level(LED_PIN, 1); 
    ESP_LOGI(tag, "Starting DDOS attack...");
	ESP_LOGI(tag, "LED current level: %d", gpio_get_level(LED_PIN));  
}

void start_packet_sniffer() {
    gpio_set_level(LED_PIN, 1); 
    ESP_LOGI(tag, "Starting packet sniffer...");
	ESP_LOGI(tag, "LED current level: %d", gpio_get_level(LED_PIN));  
}

void exit_menu() {
    gpio_set_level(LED_PIN, 0); 
    ESP_LOGI(tag, "Exiting menu...");
	ESP_LOGI(tag, "LED current level: %d", gpio_get_level(LED_PIN));  
}

MenuNode evilPortalSettings = {
	.options = {"start evil AP", "data", "options", "exit"},
	.num_options = 4,
	.children = NULL, 
	.num_children = 0,
	.parent = &wifiSettings,
	.actions = { start_evil_portal, NULL, NULL, exit_menu } // add this later for exit, and start 
};

MenuNode ddosSettings = {
	.options = {"start DDOS", "options", "exit"},
	.num_options = 3,
	.children = NULL, 
	.num_children = 0,
	.parent = &wifiSettings,
	.actions = { start_ddos_attack, NULL, exit_menu } // add this later for exit, and start 
};

MenuNode snifferSettings = {
	.options = {"start sniffing", "dump", "options", "exit"},
	.num_options = 4,
	.children = NULL,
	.num_children = 0,
	.parent = &wifiSettings,
	.actions = { start_packet_sniffer, NULL, NULL, exit_menu } // add this later for exit, and start 
};

MenuNode* wifiChildren[] = { &evilPortalSettings, &ddosSettings, &snifferSettings };

MenuNode wifiSettings = {
	.options = {"evil portal", "DDOS attack", "packet sniffer", "exit"},
	.num_options = 4,
	.children = wifiChildren,
	.num_children = 3,
	.parent = &mainMenu,
	.actions = { NULL, NULL, NULL, exit_menu } // add this later for exit, and start 
};

MenuNode bluetoothSettings = {
	.options = {"start BLE spam", "script select", "dump", "exit"},
	.num_options = 4,
	.children = NULL, 
	.num_children = 0,
	.parent = &mainMenu,
	.actions = { start_ble_spam_attack, NULL, NULL, exit_menu } // add this later for exit and start
};

MenuNode* mainMenuChildren[] = { &wifiSettings, &bluetoothSettings };

MenuNode mainMenu = {
	.options = {"WiFi", "BLE"},
	.num_options = 2,
	.children = mainMenuChildren, 
	.num_children = 2,
	.parent = NULL,
	.actions = { NULL, NULL } // stays null since this is just the head menu in tree
};

void display_menu(SSD1306_t *dev, MenuNode* currentMenu, int selected_index) {

	ssd1306_clear_screen(dev, false);
	ssd1306_contrast(dev, 0xff);

	for (int i = 0; i < currentMenu->num_options; i++) {
		char line[20];
		if(i == selected_index) {
			snprintf(line, sizeof(line), "> %s", currentMenu->options[i]);
			ssd1306_display_text(dev, i, line, strlen(line), true);
		} else {
			snprintf(line, sizeof(line), " %s", currentMenu->options[i]);
			ssd1306_display_text(dev, i, line, strlen(line), false);
		}
	}
}

bool read_button(int pin) {
	return gpio_get_level(pin) == 0; // button pressed
}

// menu traverse tree
void handle_menu_nav(SSD1306_t *dev, MenuNode** currentMenu, int* selected_index) {
	if (read_button(BUTTON_PIN1)) {
		if ((*currentMenu)->children != NULL) {
			(*currentMenu) = (*currentMenu)->children[*selected_index];
		} else if ((*currentMenu)->actions[*selected_index] != NULL) {
			(*currentMenu)->actions[*selected_index](); // run function at index

			if ((*currentMenu)->actions[*selected_index] == exit_menu) {
				(*currentMenu) = (*currentMenu)->parent;
			}
		} 
		display_menu(dev, *currentMenu, *selected_index);
		vTaskDelay(DEBOUNCE_TIME);
	}

	if (read_button(BUTTON_PIN2)) { // up button
		(*selected_index)--;
		if (*selected_index < 0) *selected_index = 0;
		display_menu(dev, *currentMenu, *selected_index);
		vTaskDelay(DEBOUNCE_TIME);
	}

	if (read_button(BUTTON_PIN3)) { // down button
		(*selected_index)++;
		if (*selected_index >= (*currentMenu)->num_options) *selected_index = (*currentMenu)->num_options -1;
		display_menu(dev, *currentMenu, *selected_index);
		vTaskDelay(DEBOUNCE_TIME);
	}

}

void app_main(void)
{
	// configure buttons
	gpio_set_direction(BUTTON_PIN1, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON_PIN1, GPIO_PULLUP_ONLY);
	gpio_set_direction(BUTTON_PIN2, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON_PIN2, GPIO_PULLUP_ONLY);
	gpio_set_direction(BUTTON_PIN3, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON_PIN3, GPIO_PULLUP_ONLY);

	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    SSD1306_t dev; // pointer to screen config

	int selected_index = 0; // current selected item

#if CONFIG_I2C_INTERFACE
	ESP_LOGI(tag, "INTERFACE is i2c");
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE

#if CONFIG_SSD1306_128x64
	ESP_LOGI(tag, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64

	// intro screen
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 0, "macero", 6, false);
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	MenuNode *currentMenu = &mainMenu;

	display_menu(&dev, currentMenu, selected_index);

	while(1) {
		handle_menu_nav(&dev, &currentMenu, &selected_index);
		vTaskDelay(1);
	}
}
