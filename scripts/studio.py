# -*- coding: utf-8 -*-
# File: studio.py
# Date: 20210707
# Author: cuiliuyi
# This file contains the Studio related action classes and methods
import os
import xml.etree.ElementTree as etree
from pathlib import Path
from mkdist import do_copy_folder, do_copy_file


class OSConfigConverter:
    """
    功能：modify the osconfig.py
    方法入口：search_update_osconfig()

    """
    IMPORT_WORDS = ['import json\n',
                    '\n',
                    '\n']
    # method of read json
    UPDATE_WORDS = ['\n',
                    '\n',
                    'def get_params_from_json():\n',
                    '    """\n',
                    '    read json and update local variables\n',
                    '    Returns: None\n',
                    '\n',
                    '    """\n',
                    "    file_name = 'osconfig.json'\n",
                    '    if not os.path.exists(file_name):\n',
                    '        return\n',
                    "    with open(file_name, 'r') as fp:\n",
                    '        json_values = json.load(fp)\n',
                    '\n',
                    "    assert len(json_values) == 1, 'osconfig.json must have only one key-value pair, please check it'\n",
                    '    cross_tool = list(json_values.keys())[0]\n',
                    '    globals().update(json_values[cross_tool][cross_tool])\n',
                    "    globals().update({'CROSS_TOOL': cross_tool})\n",
                    '\n',
                    "    if cross_tool == 'gcc':\n",
                    "        globals().update({'COMPILER': 'gcc', 'PREFIX': PREFIX})\n",
                    "    elif cross_tool == 'keil':\n",
                    '        # Notice: The installation path of armcc cannot have Chinese\n',
                    "        globals().update({'COMPILER': 'armcc'})\n",
                    "    elif cross_tool == 'iar':\n",
                    "        globals().update({'COMPILER': 'iar'})\n",
                    '\n',
                    '\n',
                    'get_params_from_json()\n']

    @staticmethod
    def search_file(file_name: str, file_path):
        """
        search file named file_name in file_path folder
        Args:
            file_name: str
            file_path: Path() or str

        Returns:
            Generator

        """
        files = Path(file_path).rglob(file_name)
        return files

    @staticmethod
    def read_osconfig(file_path) -> list:
        """
        read osconfig.py, insert function and update
        Args:
            file_path: str/Path

        Returns:
            list
        """
        new_osconfig = []
        import_insert_flag = False
        with open(file_path, 'r') as fp_read:
            for _line in fp_read:
                if _line.startswith('import'):
                    new_osconfig.append(_line)
                elif not import_insert_flag:  # insert new import line
                    new_osconfig.extend(OSConfigConverter.IMPORT_WORDS)
                    import_insert_flag = True
                else:
                    new_osconfig.append(_line)
            new_osconfig.extend(OSConfigConverter.UPDATE_WORDS)
        return new_osconfig

    @staticmethod
    def update_osconfig(file_path) -> None:
        """

        Args:
            file_path: str/Path

        Returns:
            None
        """
        new_osconfig_words = OSConfigConverter.read_osconfig(file_path)

        with open(file_path, 'w') as fp_write:
            for _line in new_osconfig_words:
                fp_write.write(_line)

    @staticmethod
    def search_update_osconfig(src_path, target_file_name='osconfig.py'):
        """
        search and update osconfig.py
        Args:
            src_path: str/Path, folder path
            target_file_name: str
        Returns:

        """
        files = OSConfigConverter.search_file(target_file_name, src_path)
        for _file in files:
            OSConfigConverter.update_osconfig(_file)


class StudioProjectInit:
    """
    Improve the content of project initialization according to the logic of the studio
    """
    SOURCE_BASE_PATH = Path('../templates/studio')

    def __call__(self, *args, **kwargs):
        self.init_project(*args)

    @staticmethod
    def init_project(project_path: str) -> None:
        OSConfigConverter.search_update_osconfig(project_path)
        project_name = Path(project_path).name
        StudioProjectInit.copy_(project_path)
        StudioProjectInit.update_cproject_file(project_path, project_name)
        StudioProjectInit.update_project_file(project_path, project_name)
        StudioProjectInit.update_launch(project_path)

    @staticmethod
    def copy_(project_path: str) -> None:
        """
        copy file or folder from ../templates/studio to project
        """
        to_copy_files = ('.cproject', '.project', '.settings')

        for _file in to_copy_files:
            file_path = os.path.join(project_path, _file)
            source_path = os.path.join(
                StudioProjectInit.SOURCE_BASE_PATH, _file)
            assert os.path.exists(source_path), 'os'
            assert Path(source_path).exists(), 'Path'
            if Path(source_path).is_file():
                do_copy_file(source_path, file_path)
            elif Path(source_path).is_dir():
                do_copy_folder(source_path, file_path)

    @staticmethod
    def update_cproject_file(project_path: str, project_name) -> None:
        """

        Args:
            project_path:
            project_name:

        Returns:

        Examples:
            project_name: stm32l475-atk-pandora

        """
        file_path = os.path.join(project_path, '.cproject')
        cproject = etree.parse(file_path).getroot()
        configurations = cproject.findall('storageModule/configuration')
        for configuration in configurations:
            resource = configuration.find('resource')
            resource.set('resourceType', 'PROJECT')
            resource.set('workspacePath', f'/{project_name}')

        # write back to .cproject
        out = open(file_path, 'wb')
        out.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'.encode('UTF-8'))
        out.write('<?fileVersion 4.0.0?>'.encode('utf-8'))
        out.write(etree.tostring(cproject, encoding='utf-8'))
        out.close()

    @staticmethod
    def update_project_file(project_path: str, project_name: str) -> None:
        """

        Args:
            project_path:
            project_name:

        Returns:

        Examples:
            project_name: stm32l475-atk-pandora

        """
        file_path = os.path.join(project_path, '.project')
        project = etree.parse(file_path).getroot()
        project.find("name").text = project_name
        # links = project.findall('linkedResources/link')
        # for link in links:
        #     folder_item = link.find("name").text
        #     if folder_item == "oneos":
        #         continue
        #     folder_name = folder_item.split("/")[-1]
        #     link.find("location").text = os.path.abspath(os.path.join(bsp_path, folder_name))

        out = open(file_path, 'wb')
        out.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'.encode('UTF-8'))
        out.write(etree.tostring(project, encoding='utf-8'))
        out.close()

    @staticmethod
    def update_launch(project_path: str) -> None:
        project_name = Path(project_path).name
        launch_files = StudioProjectInit.update_launch_filename(project_path)
        for file in launch_files:
            uvprojx = etree.parse(os.path.join(project_path, "project.uvprojx")).getroot()
            device = uvprojx.find("Targets/Target/TargetOption/TargetCommonOption/Device").text
            launch = etree.parse(file).getroot()
            for element in launch.findall("stringAttribute"):
                if element.get("key") in ("ilg.gnumcueclipse.debug.gdbjtag.jlink.flashDeviceName",
                                          "ilg.gnumcueclipse.debug.gdbjtag.jlink.gdbServerDeviceName"):
                    element.set("value", device)
                if element.get("key") == "org.eclipse.cdt.launch.PROJECT_ATTR":
                    element.set("value", project_name)
            for ele in launch.findall("listAttribute"):
                if ele.get("key") == "org.eclipse.debug.core.MAPPED_RESOURCE_PATHS":
                    ele.find("listEntry").set("value", "/" + project_name)

            out = open(file, 'wb')
            out.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'.encode('UTF-8'))
            out.write(etree.tostring(launch, encoding='utf-8'))
            out.close()

    @staticmethod
    def update_launch_filename(project_path: str) -> list:
        """
        rename launch files
        Args:
            project_path:

        Returns:

        Examples:
            projectName.JLink.Debug.oneoslaunch  -> stm32l475-atk-pandora.JLink.Debug.oneoslaunch

        """
        project_name = Path(project_path).name
        launch_files = ('projectName.JLink.Debug.oneoslaunch',
                        'projectName.STLink.Debug.oneoslaunch')
        new_launch_files = []
        for file in launch_files:
            new_file_name = file.replace('projectName', project_name)
            file_path = os.path.join(project_path, '.settings', file)
            new_file_path = os.path.join(project_path, '.settings', new_file_name)
            os.rename(file_path, new_file_path)
            new_launch_files.append(new_file_path)
        return new_launch_files


init_project = StudioProjectInit()  # 和之前的调用方式保持一致
