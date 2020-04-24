include_guard()

# Директория для генерируемых исходных файлов
set(XRAD_Project_GeneratedDir ${CMAKE_CURRENT_BINARY_DIR}/Generated)

# Файл pre.h, расположение по умолчанию в ${XRAD_Project_GeneratedDir}
set(XRAD_Project_Generated_pre_h
	${XRAD_Project_GeneratedDir}/pre.h
	)
