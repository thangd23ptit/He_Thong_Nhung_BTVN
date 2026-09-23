#include "FAT32.h"
#include "SD.h"
#include "UART.h"

static uint8_t sector_buffer[512];

static uint32_t partition_start;
static uint16_t bytes_per_sector;
static uint8_t sectors_per_cluster;
static uint16_t reserved_sectors;
static uint8_t number_of_fats;
static uint32_t fat_size;
static uint32_t root_cluster;
static uint32_t fat_begin_lba;
static uint32_t data_begin_lba;
static uint32_t total_sectors;
static uint32_t total_clusters;

static uint16_t FAT32_Read16(
    const uint8_t *buffer
)
{
    return
        ((uint16_t)buffer[0]) |
        ((uint16_t)buffer[1] << 8);
}

static uint32_t FAT32_Read32(
    const uint8_t *buffer
)
{
    return
        ((uint32_t)buffer[0]) |
        ((uint32_t)buffer[1] << 8) |
        ((uint32_t)buffer[2] << 16) |
        ((uint32_t)buffer[3] << 24);
}

static void FAT32_Write32(
    uint8_t *buffer,
    uint32_t value
)
{
    buffer[0] = (uint8_t)value;
    buffer[1] = (uint8_t)(value >> 8);
    buffer[2] = (uint8_t)(value >> 16);
    buffer[3] = (uint8_t)(value >> 24);
}

static uint32_t FAT32_ClusterToSector(
    uint32_t cluster
)
{
    return
        data_begin_lba +
        ((cluster - 2UL) * sectors_per_cluster);
}

static uint32_t FAT32_GetNextCluster(
    uint32_t cluster
)
{
    uint32_t fat_offset;
    uint32_t sector;
    uint16_t offset;
    uint32_t value;

    fat_offset = cluster * 4UL;

    sector =
        fat_begin_lba +
        (fat_offset / bytes_per_sector);

    offset =
        (uint16_t)(fat_offset % bytes_per_sector);

    if (SD_ReadBlock(
            sector,
            sector_buffer
        ) != SD_OK)
    {
        return FAT32_BAD;
    }

    value =
        FAT32_Read32(
            &sector_buffer[offset]
        );

    value &= 0x0FFFFFFFUL;

    return value;
}

static uint32_t FAT32_FindFreeCluster(void)
{
    uint32_t fat_sector;
    uint16_t offset;
    uint32_t cluster;
    uint32_t value;

    UART1_SendString(
        "SEARCH FREE CLUSTER...\r\n"
    );

    for (
        fat_sector = fat_begin_lba;
        fat_sector < fat_begin_lba + fat_size;
        fat_sector++
    )
    {
        if (SD_ReadBlock(
                fat_sector,
                sector_buffer
            ) != SD_OK)
        {
            UART1_SendString(
                "FAT READ ERROR\r\n"
            );

            return 0;
        }

        for (
            offset = 0;
            offset < 512;
            offset += 4
        )
        {
            cluster =
                ((fat_sector - fat_begin_lba) * 128UL) +
                ((uint32_t)offset / 4UL);

            if (cluster < 2)
            {
                continue;
            }

            if (cluster >= total_clusters + 2UL)
            {
                return 0;
            }

            value =
                FAT32_Read32(
                    &sector_buffer[offset]
                );

            value &= 0x0FFFFFFFUL;

            if (value == 0)
            {
                UART1_SendString(
                    "FREE CLUSTER: "
                );

                UART1_SendNumber(cluster);

                UART1_SendString("\r\n");

                return cluster;
            }
        }
    }

    UART1_SendString(
        "NO FREE CLUSTER\r\n"
    );

    return 0;
}

static uint8_t FAT32_SetCluster(
    uint32_t cluster,
    uint32_t value
)
{
    uint32_t fat_offset;
    uint32_t sector;
    uint16_t offset;
    uint8_t fat;
    uint32_t old_value;

    if (cluster < 2)
    {
        return FAT32_ERROR;
    }

    if (cluster >= total_clusters + 2UL)
    {
        return FAT32_ERROR;
    }

    fat_offset = cluster * 4UL;

    sector =
        fat_begin_lba +
        (fat_offset / bytes_per_sector);

    offset =
        (uint16_t)(fat_offset % bytes_per_sector);

    if (SD_ReadBlock(
            sector,
            sector_buffer
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }

    old_value =
        FAT32_Read32(
            &sector_buffer[offset]
        );

    value =
        (old_value & 0xF0000000UL) |
        (value & 0x0FFFFFFFUL);

    FAT32_Write32(
        &sector_buffer[offset],
        value
    );

    for (
        fat = 0;
        fat < number_of_fats;
        fat++
    )
    {
        uint32_t fat_sector;

        fat_sector =
            sector +
            ((uint32_t)fat * fat_size);

        if (SD_WriteBlock(
                fat_sector,
                sector_buffer
            ) != SD_OK)
        {
            return FAT32_ERROR;
        }
    }

    return FAT32_OK;
}

static void FAT32_MakeShortName(
    const char *filename,
    uint8_t *shortname
)
{
    uint8_t i;
    uint8_t j;
    uint8_t pos;
    char c;

    for (i = 0; i < 11; i++)
    {
        shortname[i] = ' ';
    }

    i = 0;
    j = 0;

    while (
        filename[i] != '\0' &&
        filename[i] != '.' &&
        j < 8
    )
    {
        c = filename[i];

        if (c >= 'a' && c <= 'z')
        {
            c -= 32;
        }

        shortname[j++] = (uint8_t)c;

        i++;
    }

    while (
        filename[i] != '\0' &&
        filename[i] != '.' &&
        filename[i] != '\r' &&
        filename[i] != '\n'
    )
    {
        i++;
    }

    if (filename[i] == '.')
    {
        i++;
        j = 8;

        while (
            filename[i] != '\0' &&
            j < 11
        )
        {
            c = filename[i];

            if (c >= 'a' && c <= 'z')
            {
                c -= 32;
            }

            shortname[j++] = (uint8_t)c;

            i++;
        }
    }
}

static uint8_t FAT32_NameEqual(
    const uint8_t *entry,
    const uint8_t *name
)
{
    uint8_t i;

    for (i = 0; i < 11; i++)
    {
        if (entry[i] != name[i])
        {
            return 0;
        }
    }

    return 1;
}

static uint8_t FAT32_FindFile(
    const char *filename,
    uint32_t *file_cluster,
    uint32_t *file_size,
    uint32_t *entry_sector,
    uint16_t *entry_offset
)
{
    uint8_t shortname[11];
    uint32_t cluster;
    uint32_t sector;
    uint16_t offset;
    uint8_t attr;
    uint32_t high;
    uint32_t low;

    FAT32_MakeShortName(
        filename,
        shortname
    );

    cluster = root_cluster;

    while (
        cluster >= 2 &&
        cluster < FAT32_EOC
    )
    {
        for (
            uint8_t s = 0;
            s < sectors_per_cluster;
            s++
        )
        {
            sector =
                FAT32_ClusterToSector(cluster) + s;

            if (SD_ReadBlock(
                    sector,
                    sector_buffer
                ) != SD_OK)
            {
                return FAT32_ERROR;
            }

            for (
                offset = 0;
                offset < 512;
                offset += 32
            )
            {
                if (
                    sector_buffer[offset] == 0x00
                )
                {
                    return FAT32_ERROR;
                }

                if (
                    sector_buffer[offset] == 0xE5
                )
                {
                    continue;
                }

                attr =
                    sector_buffer[offset + 11];

                if (attr == 0x0F)
                {
                    continue;
                }

                if (
                    FAT32_NameEqual(
                        &sector_buffer[offset],
                        shortname
                    )
                )
                {
                    high =
                        FAT32_Read16(
                            &sector_buffer[offset + 20]
                        );

                    low =
                        FAT32_Read16(
                            &sector_buffer[offset + 26]
                        );

                    *file_cluster =
                        (high << 16) | low;

                    *file_size =
                        FAT32_Read32(
                            &sector_buffer[offset + 28]
                        );

                    *entry_sector = sector;
                    *entry_offset = offset;

                    return FAT32_OK;
                }
            }
        }

        cluster =
            FAT32_GetNextCluster(cluster);

        if (
            cluster == FAT32_BAD ||
            cluster >= FAT32_EOC
        )
        {
            break;
        }
    }

    return FAT32_ERROR;
}

static uint8_t FAT32_FindFreeEntry(
    uint32_t *entry_sector,
    uint16_t *entry_offset
)
{
    uint32_t cluster;
    uint32_t sector;
    uint16_t offset;

    cluster = root_cluster;

    while (
        cluster >= 2 &&
        cluster < FAT32_EOC
    )
    {
        for (
            uint8_t s = 0;
            s < sectors_per_cluster;
            s++
        )
        {
            sector =
                FAT32_ClusterToSector(cluster) + s;

            if (SD_ReadBlock(
                    sector,
                    sector_buffer
                ) != SD_OK)
            {
                return FAT32_ERROR;
            }

            for (
                offset = 0;
                offset < 512;
                offset += 32
            )
            {
                if (
                    sector_buffer[offset] == 0x00 ||
                    sector_buffer[offset] == 0xE5
                )
                {
                    *entry_sector = sector;
                    *entry_offset = offset;

                    return FAT32_OK;
                }
            }
        }

        cluster =
            FAT32_GetNextCluster(cluster);

        if (
            cluster == FAT32_BAD ||
            cluster >= FAT32_EOC
        )
        {
            break;
        }
    }

    return FAT32_ERROR;
}

static uint8_t FAT32_CreateFileInternal(
    const char *filename,
    uint32_t *new_cluster
)
{
    uint8_t shortname[11];
    uint32_t entry_sector;
    uint16_t entry_offset;
    uint32_t cluster;
    uint8_t i;

    if (FAT32_FindFile(
            filename,
            &cluster,
            &cluster,
            &entry_sector,
            &entry_offset
        ) == FAT32_OK)
    {
        *new_cluster = cluster;

        return FAT32_OK;
    }

    if (FAT32_FindFreeEntry(
            &entry_sector,
            &entry_offset
        ) != FAT32_OK)
    {
        return FAT32_ERROR;
    }

    cluster =
        FAT32_FindFreeCluster();

    if (cluster == 0)
    {
        return FAT32_ERROR;
    }

    if (FAT32_SetCluster(
            cluster,
            FAT32_EOC
        ) != FAT32_OK)
    {
        return FAT32_ERROR;
    }

    if (SD_ReadBlock(
            entry_sector,
            sector_buffer
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }

    FAT32_MakeShortName(
        filename,
        shortname
    );

    for (i = 0; i < 32; i++)
    {
        sector_buffer[entry_offset + i] = 0;
    }

    for (i = 0; i < 11; i++)
    {
        sector_buffer[
            entry_offset + i
        ] = shortname[i];
    }

    sector_buffer[entry_offset + 11] = 0x20;

    sector_buffer[entry_offset + 20] = 0;
    sector_buffer[entry_offset + 21] = 0;

    sector_buffer[entry_offset + 26] =
        (uint8_t)cluster;

    sector_buffer[entry_offset + 27] =
        (uint8_t)(cluster >> 8);

    sector_buffer[entry_offset + 28] = 0;
    sector_buffer[entry_offset + 29] = 0;
    sector_buffer[entry_offset + 30] = 0;
    sector_buffer[entry_offset + 31] = 0;

    if (SD_WriteBlock(
            entry_sector,
            sector_buffer
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }

    *new_cluster = cluster;

    return FAT32_OK;
}

uint8_t FAT32_Init(void)
{
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint32_t root;

    if (SD_ReadBlock(
            0,
            sector_buffer
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }

    if (
        sector_buffer[510] != 0x55 ||
        sector_buffer[511] != 0xAA
    )
    {
        return FAT32_ERROR;
    }

    partition_start = 0;

    if (
        sector_buffer[0] == 0xEB ||
        sector_buffer[0] == 0xE9
    )
    {
        bytes_per_sector =
            FAT32_Read16(
                &sector_buffer[11]
            );

        sectors_per_cluster =
            sector_buffer[13];

        reserved_sectors =
            FAT32_Read16(
                &sector_buffer[14]
            );

        number_of_fats =
            sector_buffer[16];

        total_sectors_32 =
            FAT32_Read32(
                &sector_buffer[32]
            );

        fat_size_32 =
            FAT32_Read32(
                &sector_buffer[36]
            );

        root =
            FAT32_Read32(
                &sector_buffer[44]
            );
    }
    else
    {
        uint32_t partition;

        partition =
            FAT32_Read32(
                &sector_buffer[454]
            );

        partition_start = partition;

        if (SD_ReadBlock(
                partition_start,
                sector_buffer
            ) != SD_OK)
        {
            return FAT32_ERROR;
        }

        if (
            sector_buffer[510] != 0x55 ||
            sector_buffer[511] != 0xAA
        )
        {
            return FAT32_ERROR;
        }

        bytes_per_sector =
            FAT32_Read16(
                &sector_buffer[11]
            );

        sectors_per_cluster =
            sector_buffer[13];

        reserved_sectors =
            FAT32_Read16(
                &sector_buffer[14]
            );

        number_of_fats =
            sector_buffer[16];

        total_sectors_32 =
            FAT32_Read32(
                &sector_buffer[32]
            );

        fat_size_32 =
            FAT32_Read32(
                &sector_buffer[36]
            );

        root =
            FAT32_Read32(
                &sector_buffer[44]
            );
    }

    if (bytes_per_sector != 512)
    {
        return FAT32_ERROR;
    }

    if (sectors_per_cluster == 0)
    {
        return FAT32_ERROR;
    }

    if (number_of_fats == 0)
    {
        return FAT32_ERROR;
    }

    if (fat_size_32 == 0)
    {
        return FAT32_ERROR;
    }

    fat_size = fat_size_32;

    total_sectors = total_sectors_32;

    root_cluster = root;

    fat_begin_lba =
        partition_start +
        reserved_sectors;

    data_begin_lba =
        fat_begin_lba +
        ((uint32_t)number_of_fats * fat_size);

    if (total_sectors <=
        reserved_sectors +
        ((uint32_t)number_of_fats * fat_size))
    {
        return FAT32_ERROR;
    }

    total_clusters =
        (
            total_sectors -
            reserved_sectors -
            ((uint32_t)number_of_fats * fat_size)
        ) /
        sectors_per_cluster;

    return FAT32_OK;
}

uint8_t FAT32_CreateFile(
    const char *filename
)
{
    uint32_t cluster;

    if (FAT32_CreateFileInternal(
            filename,
            &cluster
        ) != FAT32_OK)
    {
        return FAT32_ERROR;
    }

    return FAT32_OK;
}

uint8_t FAT32_AppendFile(
    const char *filename,
    const char *data,
    uint32_t length
)
{
    uint32_t file_cluster;
    uint32_t file_size;
    uint32_t entry_sector;
    uint16_t entry_offset;
    uint32_t cluster;
    uint32_t next_cluster;
    uint32_t cluster_index;
    uint32_t byte_offset;
    uint32_t remaining;
    uint32_t write_count;
    uint32_t sector;
    uint32_t sector_offset;

    UART1_SendString(
        "WRITE "
    );

    UART1_SendString(
        filename
    );

    UART1_SendString(
        "...\r\n"
    );

    if (FAT32_FindFile(
            filename,
            &file_cluster,
            &file_size,
            &entry_sector,
            &entry_offset
        ) != FAT32_OK)
    {
        UART1_SendString(
            "FILE NOT FOUND\r\n"
        );

        if (FAT32_CreateFileInternal(
                filename,
                &file_cluster
            ) != FAT32_OK)
        {
            UART1_SendString(
                "CREATE FILE ERROR\r\n"
            );

            return FAT32_ERROR;
        }

        file_size = 0;

        if (FAT32_FindFile(
                filename,
                &file_cluster,
                &file_size,
                &entry_sector,
                &entry_offset
            ) != FAT32_OK)
        {
            return FAT32_ERROR;
        }
    }

    if (length == 0)
    {
        return FAT32_OK;
    }

    cluster = file_cluster;

    cluster_index =
        file_size /
        ((uint32_t)bytes_per_sector *
         sectors_per_cluster);

    for (
        uint32_t i = 0;
        i < cluster_index;
        i++
    )
    {
        next_cluster =
            FAT32_GetNextCluster(cluster);

        if (
            next_cluster == FAT32_BAD ||
            next_cluster >= FAT32_EOC
        )
        {
            next_cluster =
                FAT32_FindFreeCluster();

            if (next_cluster == 0)
            {
                return FAT32_ERROR;
            }

            if (FAT32_SetCluster(
                    cluster,
                    next_cluster
                ) != FAT32_OK)
            {
                return FAT32_ERROR;
            }

            if (FAT32_SetCluster(
                    next_cluster,
                    FAT32_EOC
                ) != FAT32_OK)
            {
                return FAT32_ERROR;
            }

            cluster = next_cluster;
        }
        else
        {
            cluster = next_cluster;
        }
    }

    byte_offset =
        file_size %
        (
            (uint32_t)bytes_per_sector *
            sectors_per_cluster
        );

    remaining = length;

    while (remaining)
    {
        sector =
            FAT32_ClusterToSector(cluster) +
            (
                byte_offset /
                bytes_per_sector
            );

        sector_offset =
            byte_offset %
            bytes_per_sector;

        if (SD_ReadBlock(
                sector,
                sector_buffer
            ) != SD_OK)
        {
            return FAT32_ERROR;
        }

        write_count =
            bytes_per_sector -
            sector_offset;

        if (write_count > remaining)
        {
            write_count = remaining;
        }

        for (
            uint32_t i = 0;
            i < write_count;
            i++
        )
        {
            sector_buffer[
                sector_offset + i
            ] = (uint8_t)data[i];
        }

        if (SD_WriteBlock(
                sector,
                sector_buffer
            ) != SD_OK)
        {
            return FAT32_ERROR;
        }

        data += write_count;
        remaining -= write_count;
        file_size += write_count;
        byte_offset += write_count;

        if (
            byte_offset >=
            (
                (uint32_t)bytes_per_sector *
                sectors_per_cluster
            )
        )
        {
            byte_offset = 0;

            next_cluster =
                FAT32_GetNextCluster(cluster);

            if (
                next_cluster == FAT32_BAD ||
                next_cluster >= FAT32_EOC
            )
            {
                next_cluster =
                    FAT32_FindFreeCluster();

                if (next_cluster == 0)
                {
                    return FAT32_ERROR;
                }

                if (FAT32_SetCluster(
                        cluster,
                        next_cluster
                    ) != FAT32_OK)
                {
                    return FAT32_ERROR;
                }

                if (FAT32_SetCluster(
                        next_cluster,
                        FAT32_EOC
                    ) != FAT32_OK)
                {
                    return FAT32_ERROR;
                }

                cluster = next_cluster;
            }
            else
            {
                cluster = next_cluster;
            }
        }
    }

    if (SD_ReadBlock(
            entry_sector,
            sector_buffer
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }

    FAT32_Write32(
        &sector_buffer[
            entry_offset + 28
        ],
        file_size
    );

    if (SD_WriteBlock(
            entry_sector,
            sector_buffer
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }

    UART1_SendString(
        "WRITE OK\r\n"
    );

    return FAT32_OK;
}

uint8_t FAT32_ReadFile(
    const char *filename
)
{
    uint32_t file_cluster;
    uint32_t file_size;
    uint32_t entry_sector;
    uint16_t entry_offset;
    uint32_t cluster;
    uint32_t remaining;
    uint32_t sector;
    uint16_t i;

    if (FAT32_FindFile(
            filename,
            &file_cluster,
            &file_size,
            &entry_sector,
            &entry_offset
        ) != FAT32_OK)
    {
        UART1_SendString(
            "FILE NOT FOUND\r\n"
        );

        return FAT32_ERROR;
    }

    if (file_size == 0)
    {
        UART1_SendString(
            "FILE EMPTY\r\n"
        );

        return FAT32_OK;
    }

    UART1_SendString(
        "\r\n"
    );

    cluster = file_cluster;
    remaining = file_size;

    while (
        remaining &&
        cluster >= 2 &&
        cluster < FAT32_EOC
    )
    {
        for (
            uint8_t s = 0;
            s < sectors_per_cluster;
            s++
        )
        {
            sector =
                FAT32_ClusterToSector(cluster) + s;

            if (SD_ReadBlock(
                    sector,
                    sector_buffer
                ) != SD_OK)
            {
                return FAT32_ERROR;
            }

            for (
                i = 0;
                i < 512 && remaining;
                i++
            )
            {
                UART1_SendChar(
                    (char)sector_buffer[i]
                );

                remaining--;
            }
        }

        if (remaining == 0)
        {
            break;
        }

        cluster =
            FAT32_GetNextCluster(cluster);

        if (
            cluster == FAT32_BAD ||
            cluster >= FAT32_EOC
        )
        {
            break;
        }
    }

    UART1_SendString(
        "\r\n"
    );

    return FAT32_OK;
}

void FAT32_PrintStatus(void)
{
    UART1_SendString(
        "\r\n"
        "FAT32 STATUS\r\n"
    );

    UART1_SendString(
        "PARTITION START: "
    );

    UART1_SendNumber(
        partition_start
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTES/SECTOR: "
    );

    UART1_SendNumber(
        bytes_per_sector
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "SECTORS/CLUSTER: "
    );

    UART1_SendNumber(
        sectors_per_cluster
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "RESERVED: "
    );

    UART1_SendNumber(
        reserved_sectors
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "FATS: "
    );

    UART1_SendNumber(
        number_of_fats
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "FAT SIZE: "
    );

    UART1_SendNumber(
        fat_size
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "ROOT CLUSTER: "
    );

    UART1_SendNumber(
        root_cluster
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "FAT START: "
    );

    UART1_SendNumber(
        fat_begin_lba
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "DATA START: "
    );

    UART1_SendNumber(
        data_begin_lba
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "TOTAL CLUSTERS: "
    );

    UART1_SendNumber(
        total_clusters
    );

    UART1_SendString(
        "\r\n"
    );
}