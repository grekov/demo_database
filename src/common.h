#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @author Sviatoslav
 * @brief Common structs for client and server.
 */

enum DB_CMD_TYPE {
        DB_CMD_PUT,     /**< Put key value      */
        DB_CMD_GET,     /**< Get value by key   */
        DB_CMD_ERASE,   /**< Erase value by key */
        DB_CMD_LIST,    /**< Get list of all values */
        DB_CMD_RESP     /**< Server resonse command */
};

/**
 * @brief Command header for send.
 */
struct s_command {
        uint32_t type;          /**< Command type        */
        uint32_t len;           /**< Command length include header and data */
        uint32_t key_size;      /**< Key data size   */
        uint32_t val_size;      /**< Value data size */
};

/**
 * @brief Message struct
 */
struct s_message {
        struct s_command cmd;   /**< Received cmd data          */
        uint8_t *key;           /**< Key data, if exists.       */
        uint8_t *val;           /**< Value data, if exists      */
        uint32_t key_len;       /**< Read key length            */
        uint32_t val_len;       /**< Read value length          */
        uint32_t cmd_len;       /**< Read cmd length            */
        int sd; /**< Socket descriptor */
};

#endif /* COMMON_H */
