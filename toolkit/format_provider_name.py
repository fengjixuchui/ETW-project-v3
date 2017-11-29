import re

input_file = open("provider_name.txt", 'r')
output_file = open("formatted_provider_name.txt", 'w')

provider_name_list = input_file.readlines()
formatted_provider_name_list = []

nonalphabet = re.compile("[^a-zA-Z]+")
for provider_name in provider_name_list:
    formatted_provider_name = nonalphabet.sub("_", provider_name)
    formatted_provider_name_list.append(formatted_provider_name[0:-1] + "\n")

output_file.writelines(formatted_provider_name_list)

