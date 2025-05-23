import fs from '@ohos.file.fs';
import hilog from '@ohos.hilog';
import zlib from '@ohos.zlib';
import common from '@ohos.app.ability.common';

const TAG: string = 'TexResFileUtils';
const DOMAIN: number = 0xFF00;

/**
 * 文件工具类，用于处理文件操作
 */
export class TexResFileUtils {
  static filePath: string = '';

  /**
   * 将rawfile中的文件res.zip拷贝到沙箱环境并解压缩
   *
   * @param context 应用上下文
   * @param rawFileName 原始文件名
   * @param targetDir 目标目录名
   * @returns 解压后的目录路径
   */
  static async copyAndUnzipRawFile(context: common.UIAbilityContext, rawFileName: string = 'res.zip',
    targetDir: string = 'tex'): Promise<string> {
    try {
      // 获取应用沙箱路径
      const filesDir = context.filesDir;

      // 创建目标目录
      const targetPath = `${filesDir}/${targetDir}`;
      try {
        const stat = await fs.stat(targetPath);
        if (stat.isDirectory()) {
          hilog.info(DOMAIN, TAG, `目标目录 ${targetPath} 已存在`);
          // return targetPath;
        }
      } catch (err) {
        // 目录不存在，创建目录
        await fs.mkdir(targetPath);
        hilog.info(DOMAIN, TAG, `创建目标目录 ${targetPath} 成功`);
      }

      // 获取资源管理器
      const resourceMgr = context.resourceManager;

      // 从rawfile读取zip文件
      const zipBuffer = await resourceMgr.getRawFileContent(rawFileName);
      const zipPath = `${filesDir}/${rawFileName}`;

      // 将zip文件写入沙箱
      const zipFile = await fs.open(zipPath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
      await fs.write(zipFile.fd, zipBuffer.buffer);
      await fs.close(zipFile.fd);
      hilog.info(DOMAIN, TAG, `将 ${rawFileName} 写入沙箱成功`);

      // 解压文件
      await zlib.decompressFile(zipPath, targetPath);
      hilog.info(DOMAIN, TAG, `解压 ${rawFileName} 到 ${targetPath} 成功`);

      // 解压完成后删除zip文件
      await fs.unlink(zipPath);
      hilog.info(DOMAIN, TAG, `删除临时zip文件 ${zipPath} 成功`);

      return targetPath;
    } catch (error) {
      hilog.error(DOMAIN, TAG, `复制并解压文件失败: ${JSON.stringify(error)}`);
      return '';
    }
  }

  /**
   * 列出指定目录下的所有文件和子目录
   *
   * @param dirPath 目录路径
   * @param recursive 是否递归列出子目录中的文件，默认为true
   * @returns 文件和目录列表
   */
  static async listFiles(dirPath: string, recursive: boolean = false): Promise<Array<string>> {
    try {
      // 检查路径是否存在
      const stat = await fs.stat(dirPath);
      if (!stat.isDirectory()) {
        hilog.error(DOMAIN, TAG, `${dirPath} 不是一个目录`);
        return [];
      }

      const fileList: Array<string> = [];

      // 获取目录下的所有文件和子目录
      const files = await fs.listFile(dirPath);

      for (const fileName of files) {
        const fullPath = `${dirPath}/${fileName}`;
        fileList.push(fullPath);

        // 如果需要递归，则检查是否为目录并递归获取子目录内容
        if (recursive) {
          try {
            const fileStat = await fs.stat(fullPath);
            if (fileStat.isDirectory()) {
              TexResFileUtils.listFiles(fullPath, recursive);
              // const subFiles = await TexResFileUtils.listFiles(fullPath, recursive);
              // fileList.push(...subFiles);
            }
          } catch (err) {
            hilog.error(DOMAIN, TAG, `获取文件状态失败: ${JSON.stringify(err)}`);
          }
        }
      }

      hilog.info(DOMAIN, TAG, `成功列出 ${dirPath} 目录下的文件，共 ${fileList.length} 个`);
      return fileList;
    } catch (error) {
      hilog.error(DOMAIN, TAG, `列出目录 ${dirPath} 下的文件失败: ${JSON.stringify(error)}`);
      return [];
    }
  }
}