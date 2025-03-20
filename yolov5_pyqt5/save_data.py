import pandas as pd
import csv
import os
import shutil


def move_file(source_path, destination_path):
    shutil.move(source_path, destination_path)


def auto_adjust(data_frame, file_name):
    # 自动调整列宽，保存为excel表格
    writer = pd.ExcelWriter(file_name, engine='xlsxwriter')
    # 写excel文件使用pandas to_excel
    data_frame.to_excel(writer, startrow=1, sheet_name='Sheet1', index=False)
    # workbook = writer.book
    worksheet = writer.sheets['Sheet1']
    # 遍历每一列并设置width ==该列的最大长度。填充长度也增加了4
    for i, col in enumerate(data_frame.columns):
        # 求列I的长度
        column_len = data_frame[col].astype(str).str.len().max()
        # 如果列标题较大，则设置长度大于最大列值长度
        column_len = max(column_len, len(col)) + 4
        # 设置列的长度
        worksheet.set_column(i, i, column_len)
    writer.close()


def get_milk_data(milk_file):
    # 初始化一个字典来存储结果
    result = {}
    # 假设lines是你从txt文件中读取的行列表
    with open(milk_file, 'r') as f:
        lines = f.readlines()

    # 将编号相同的数据累加
    for line in lines:
        # 分割每一行的数据
        parts = line.split(":")
        key = parts[0].strip()
        values = list(map(int, parts[1].strip().split()))
        # 如果key在结果字典中，就把值相加
        if key in result:
            result[key] = [sum(x) for x in zip(result[key], values)]
        else:
            result[key] = values

    # 把结果覆写回文件
    with open(milk_file, 'w') as f:
        for key, values in result.items():
            f.write(f"{key}: {' '.join(map(str, values))}\n")

    # 读取流量计数据，打开保存数据的文件
    with open(milk_file) as f:
        lines = f.readlines()
        lines = [line.strip() for line in lines]  # 去掉换行符

    # for line in lines:
    #     # 分割每一行的数据
    #     parts = line.split(":")
    #     key = parts[0].strip()
    #     values = list(map(int, parts[1].strip().split()))
    #     # 如果key在结果字典中，就把值相加
    #     if key in result:
    #         result[key] = [sum(x) for x in zip(result[key], values)]
    #     else:
    #         result[key] = values

    # 有4个流量计数据(4个乳头)，原来的是直接把这个当成一个字符串存的
    # 修改成一个数组，每个乳头单独存一个，方便后期累加
    # 同时表头也修改成：流量计1、流量计2、流量计3、流量计4、总流量
    milk_dict = {}
    milk_sum = {}
    for line in lines:
        split_line = line.split(": ")
        # split_line[0]: 'cow_1001'  split_line[1]: '10 10 10 10 '
        milk_dict[split_line[0]] = split_line[1]
        # 计算总流量
        num_list = split_line[1].split()
        num_list_new = [int(i) for i in num_list]
        milk_sum[split_line[0]] = sum(num_list_new)
    # print(milk_dict)

    # 整合评级数据和流量计数据
    df2 = pd.read_csv('scores.csv', encoding='GBK')
    # df2 = pd.read_csv('scores.csv', encoding='utf-8')
    df2['流量计单独流量'] = ''
    df2['总流量'] = ''
    for key in milk_dict.keys():
        # print(milk_dict[key])
        df2.loc[df2.奶牛编号 == key, '流量计单独流量'] = milk_dict[key]
        df2.loc[df2.奶牛编号 == key, '总流量'] = milk_sum[key]

    # df2.to_csv('results.csv', encoding='GBK', index=False)
    auto_adjust(df2, '统计数据.xlsx')
    os.remove('scores.csv')


def get_data(score_file, milk_file, flag=True):
    headers = ['奶牛编号', '乳头评级', '置信度']  # 表头
    with open(score_file, 'r', newline='') as csvfile, open('temp_with_header.csv', 'w', newline='') as new_csvfile:
        reader = csv.reader(csvfile)
        writer = csv.writer(new_csvfile)
        # 写入表头
        writer.writerow(headers)
        # 写入剩余的数据
        for row in reader:
            writer.writerow(row)

    # 按奶牛编号整理数据
    df = pd.read_csv('temp_with_header.csv', encoding='GBK')
    # df = pd.read_csv('temp_with_header.csv', encoding='utf-8')
    grouped_df = df.groupby(headers[0], as_index=False).agg(list)
    grouped_df.to_csv('scores.csv', encoding='GBK', index=False)
    # grouped_df.to_csv('scores.csv', encoding='utf-8', index=False)
    os.remove('temp_with_header.csv')

    path = os.path.dirname(score_file)
    if os.path.exists(milk_file):
        get_milk_data(milk_file)
        move_file('统计数据.xlsx', path)
    else:
        auto_adjust(pd.read_csv('scores.csv', encoding='GBK'), '统计数据_无流量计数据.xlsx')
        # auto_adjust(pd.read_csv('scores.csv', encoding='utf-8'), '统计数据_无流量计数据.xlsx')
        move_file('统计数据_无流量计数据.xlsx', path)
        os.remove('scores.csv')

# if __name__ == '__main__':
#     # score_file: 评分文件路径，milk_file: 流量计数据文件路径
#     # flag: 是否同时保存流量计数据，True保存，False不保存，默认为True
#     get_data(score_file='../cow_data/20240328/results/predictions.csv',
#              milk_file='../cow_data/20240328/milk_volume.txt')
