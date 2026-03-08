#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	const float StartX = -Width * 0.5f;
	const float StartY = -Height * 0.5f;

	Cells.reserve(Rows * Cols);
	for (int row = 0; row < Rows; ++row)
	{
		for (int col = 0; col < Cols; ++col)
		{
			float Left = StartX + col * CellWidth;
			float Bottom = StartY + row * CellHeight;
			Cells.emplace_back(Left, Bottom, CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	int Index = PositionToIndex(Agent.GetPosition());
	Cells[Index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	int OldIndex = PositionToIndex(OldPos);
	int NewIndex = PositionToIndex(Agent.GetPosition());

	if (OldIndex != NewIndex)
	{
		Cells[OldIndex].Agents.remove(&Agent);
		Cells[NewIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	NrOfNeighbors = 0;

	// Build a query rect around the agent
	const FVector2D AgentPos = Agent.GetPosition();
	FRect QueryRect{};
	QueryRect.Min = { AgentPos.X - QueryRadius, AgentPos.Y - QueryRadius };
	QueryRect.Max = { AgentPos.X + QueryRadius, AgentPos.Y + QueryRadius };

	for (Cell& Cell : Cells)
	{
		if (!DoRectsOverlap(QueryRect, Cell.BoundingBox))
			continue;

		for (ASteeringAgent* pOther : Cell.Agents)
		{
			if (!IsValid(pOther) || pOther == &Agent)
				continue;

			if (FVector2D::Distance(AgentPos, pOther->GetPosition()) < QueryRadius)
			{
				Neighbors[NrOfNeighbors] = pOther;
				++NrOfNeighbors;
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	for (int i = 0; i < static_cast<int>(Cells.size()); ++i)
	{
		const Cell& Cell = Cells[i];
		const auto Points = Cell.GetRectPoints();

		// Draw cell outline
		for (int j = 0; j < 4; ++j)
		{
			DrawDebugLine(pWorld,
				FVector{ Points[j], 0.f },
				FVector{ Points[(j + 1) % 4], 0.f },
				FColor::Blue, false, -1.f, 0, 5.f);
		}

		// Draw agent count at cell center
		const FVector2D Center{
			(Cell.BoundingBox.Min.X + Cell.BoundingBox.Max.X) * 0.5f,
			(Cell.BoundingBox.Min.Y + Cell.BoundingBox.Max.Y) * 0.5f
		};

		DrawDebugString(pWorld,
			FVector{ Center, 0.f },
			FString::FromInt(static_cast<int>(Cell.Agents.size())),
			nullptr, FColor::White, 0.f);
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	const float StartX = -SpaceWidth * 0.5f;
	const float StartY = -SpaceHeight * 0.5f;

	int Col = static_cast<int>((Pos.X - StartX) / CellWidth);
	int Row = static_cast<int>((Pos.Y - StartY) / CellHeight);

	Col = FMath::Clamp(Col, 0, NrOfCols - 1);
	Row = FMath::Clamp(Row, 0, NrOfRows - 1);

	return Row * NrOfCols + Col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}