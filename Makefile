all: game resource_converter
	@echo "Built everything ^_^"

game:
	@echo "Building game..."
	@./misc/build.sh -o ./build/game -DGAME_PROJECT

resource_converter:
	@echo "Building resource converter..."
	@./misc/build.sh -o ./build/resource_converter -DRESOURCE_CONVERTER_PROJECT
