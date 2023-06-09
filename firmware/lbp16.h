#ifndef LBP16_H
#define LBP16_H


typedef struct {
    uint16_t raw;
    bool write;
    bool has_addr;
    bool info_area;
    uint8_t memory_space;
    uint8_t transfer_size;
    int8_t transfer_bits;
    int8_t transfer_bytes;
    bool addr_increment;
    uint8_t transfer_count;
    int num_bytes;
} lbp16_cmd_t;


static void lbp16_decode_cmd(uint16_t raw_cmd, lbp16_cmd_t * cmd) {
    cmd->raw = raw_cmd;
    cmd->write = raw_cmd & 0x8000;
    cmd->has_addr = raw_cmd & 0x4000;
    cmd->info_area = raw_cmd & 0x2000;
    cmd->memory_space = (raw_cmd >> 10) & 0x7;
    cmd->transfer_size = (raw_cmd >> 8) & 0x3;
    cmd->addr_increment = raw_cmd & 0x0080;
    cmd->transfer_count = raw_cmd & 0x7f;

    switch (cmd->transfer_size) {
        case 0:
            cmd->transfer_bits = 8;
            cmd->transfer_bytes = 1;
            break;
        case 1:
            cmd->transfer_bits = 16;
            cmd->transfer_bytes = 2;
            break;
        case 2:
            cmd->transfer_bits = 32;
            cmd->transfer_bytes = 4;
            break;
        case 3:
            cmd->transfer_bits = 64;
            cmd->transfer_bytes = 8;
            break;
        default:
            // This should never happen.
            cmd->transfer_bits = -1;
            cmd->transfer_bytes = -1;
            break;
    }

    cmd->num_bytes = cmd->transfer_bytes * cmd->transfer_count;
}


static void lbp16_log_cmd(lbp16_cmd_t const * const cmd) {
    printf("lbp16 cmd 0x%04x\n", cmd->raw);
    printf("    write: %d\n", cmd->write);
    printf("    has_addr: %d\n", cmd->has_addr);
    printf("    info_area: %d\n", cmd->info_area);
    printf("    memory_space: %d\n", cmd->memory_space);
    printf("    transfer_size: %d (%d bytes, %d bits)\n", cmd->transfer_size, cmd->transfer_bytes, cmd->transfer_bits);
    printf("    addr_increment: %d\n", cmd->addr_increment);
    printf("    transfer_count: %d\n", cmd->transfer_count);
    printf("    num_bytes: %d\n", cmd->num_bytes);
}


#endif // LBP16_H
