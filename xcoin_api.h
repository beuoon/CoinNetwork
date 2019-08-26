#pragma once

const char *api_request(const char *endpoint, const char *post_data);

/*
- compile
g++ -o xcoin_api_client xcoin_api_client.cpp -lcurl -lcrypto

- setting
apt-get install 
libcurl4-openssl-dev
libssl-dev
*/
