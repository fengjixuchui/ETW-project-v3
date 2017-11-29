input_file = open("guid.txt",'r')
output_file = open("guid_formatted.txt",'w')

guid_list = input_file.readlines()
formatted_guid_list = []
# C100BECE-D33A-4A4B-BF23-BBEF4663D017
print("{ 0xEDD08927, 0x9CC4, 0x4E65,{ 0xB9, 0x70, 0xC2, 0x56, 0x0F, 0xB5, 0xC2, 0x89 } }")
for guid in guid_list:
    formatted_guid = "{ 0x%s, 0x%s, 0x%s,{ 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s, 0x%s } }" % (guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])
    formatted_guid_list.append(formatted_guid + "\n")

output_file.writelines(formatted_guid_list)