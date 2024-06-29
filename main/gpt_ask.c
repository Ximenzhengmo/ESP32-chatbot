#include "gpt_ask.h"

static const char *TAG = "GPT_ASK";

#define POST_DATA  "{"                                          \
                        "\"temperature\": 0.7,"                 \
                        "\"stream\": true,"                    \
                        "\"messages\": ["                       \
                            "{"                                 \
                                "\"role\": \"user\","      \
                                "\"content\": \"%s\""           \
                            "}"                                 \
                        "]"                                     \
                    "}"
#define BAIDU_API_KEY CONFIG_BAIDU_GPT_ACCESS_KEY
#define BAIDU_SECRET_KEY CONFIG_BAIDU_GPT_ACCESS_SECERT

/* 
! The `../eb-instant` url is deprecated after 2024-07-04, the new url is `../ernie-lite-8k`
#define GPT_URL "https://aip.baidubce.com/rpc/2.0/ai_custom/v1/wenxinworkshop/chat/eb-instant?access_token=%s"
*/
#define GPT_URL "https://aip.baidubce.com/rpc/2.0/ai_custom/v1/wenxinworkshop/chat/ernie-lite-8k?access_token=%s"


GPT_resTxtState state = START;
esp_http_client_handle_t client = NULL;
char raw_response_buffer[RAW_RESPONSE_BUFFER_MAX + 512];
AnsBuffer ans_buffer = {0, {0}};

void gpt_ask_init(){
    esp_http_client_config_t config = {
        .buffer_size = 2048 * 4,
    };
    char *access_token = baidu_get_access_token(BAIDU_API_KEY, BAIDU_SECRET_KEY);
    if (access_token == NULL) {
        ESP_LOGE(TAG, "Failed to get access token");
        return;
    }
    char* baidu_url_with_token = calloc(1, strlen(GPT_URL) + strlen(access_token) + 1);
    sprintf(baidu_url_with_token, GPT_URL, access_token);
    config.url = (const char*)baidu_url_with_token;
    client = esp_http_client_init(&config);
    free(baidu_url_with_token);
    free(access_token);
}

void gpt_ask_uninit(){
    esp_http_client_cleanup(client);
}

void createTTS(){
    esp_tts_task_create();
    vTaskDelete(NULL);
}

esp_err_t gpt_post_response(char* question){
    if ( question == NULL ){
        ESP_LOGE(TAG, "question is NULL");
        return ESP_FAIL;
    }
    // POST
    char post_data[2048];
    snprintf(post_data, 2048, POST_DATA, question);
    free(question);
    int post_data_len = strlen(post_data);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err;
    if ((err = esp_http_client_open(client, post_data_len)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }
    if ( esp_http_client_write(client, post_data, post_data_len) <= 0) {
        ESP_LOGE(TAG, "Failed to write data");
        return ESP_FAIL;
    }
    int content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0) {
        ESP_LOGE(TAG, "HTTP client fetch headers failed");
    } else {
        if( esp_http_client_is_chunked_response(client) ){
            ESP_LOGI(TAG, "esp_http_client_is_chunked_response");
        }else{
            ESP_LOGI(TAG, "esp_http_client_is_not_chunked_response");
        }
        resTxtState_reset();
        ansBuffer_reset();

        GPT_resTxtState last_state = START;
        bool isAns = false;
        bool hasTTScreated = false;
        while (!esp_http_client_is_complete_data_received(client)){
            int data_read = esp_http_client_read_response(client, raw_response_buffer, RAW_RESPONSE_BUFFER_MAX);
            ESP_LOGW(TAG, "raw_response len: %d", data_read);
            if ( hasTTScreated && is_esp_tts_receive_task_working() == ESP_FAIL ){
                break;
            }
            for (int i = 0; i < data_read; i++){
                last_state = state;
                state = get_gptResTxtState(state, raw_response_buffer[i]);
                if ( state == ACCEPT && last_state == FOUND_quotation3 ){
                    isAns = true;
                }
                if ( !hasTTScreated && isAns ){
                    esp_tts_task_create();
                    hasTTScreated = true;
                }
                if ( state == START && last_state == ACCEPT ){
                    isAns = false;
                    if ( hasTTScreated ) {
                        esp_tts_send_txt( strdup(ans_buffer.ans) );
                    }
                    ESP_LOGE(TAG, "ans: %s", ans_buffer.ans);
                    ansBuffer_reset();
                }
                if ( isAns ){
                    if ( raw_response_buffer[i] == 'n' && ansBuffer_top() == '\\' ){
                        ansBuffer_rewind();
                    }else {
                        ansBuffer_append(raw_response_buffer[i]);
                    }
                }
            }
        }
        esp_tts_task_destory();
        ESP_LOGE(TAG, "esp_http_client finish");
    }
    esp_http_client_close(client);
    return ESP_OK;
}

void resTxtState_reset() {
    state = START;
}

void ansBuffer_reset() {
    ans_buffer.ans_len = 0;
    ans_buffer.ans[0] = '\0';
}

void ansBuffer_append(char c) {
    ans_buffer.ans[ans_buffer.ans_len++] = c;
    ans_buffer.ans[ans_buffer.ans_len] = '\0';
}

char ansBuffer_top(){
    if ( ans_buffer.ans_len == 0 ) return '\0';
    return ans_buffer.ans[ans_buffer.ans_len - 1];
}

void ansBuffer_rewind(){
    if ( ans_buffer.ans_len == 0 ) return;
    ans_buffer.ans[ans_buffer.ans_len - 1] = '\0';
    ans_buffer.ans_len--;
}

GPT_resTxtState get_gptResTxtState(GPT_resTxtState state, char c) {
    switch (state) {
        case START:
            if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_quotation:
            if (c == 'r')
                return FOUND_r;
            else if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_r:
            if (c == 'e')
                return FOUND_e;
            else if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_e:
            if (c == 's')
                return FOUND_s;
            else if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_s:
            if (c == 'u')
                return FOUND_u;
            else if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_u:
            if (c == 'l')
                return FOUND_l;
            else if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_l:
            if (c == 't')
                return FOUND_t;
            else if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_t:
            if (c == '\"')
                return FOUND_quotation2;
            else
                return START;
        case FOUND_quotation2:
            if (c == ':')
                return FOUND_colon;
            else if (c == '\"')
                return FOUND_quotation;
            else
                return START;
        case FOUND_colon:
            if (c == '\"')
                return FOUND_quotation3;
            else
                return START;
        case FOUND_quotation3:
        case ACCEPT:
            if (c == '\"')
                return START;
            else
                return ACCEPT;
        default:
            return state;
    }
}
